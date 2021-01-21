
#include <sstream>
#include <vector>

#include <tws/Contract.h>

#include "client.h"

#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace pybind11::literals;

using clock_type = std::chrono::system_clock;

// creator function bind logger to py::print
std::unique_ptr<client> create_client(int id, const std::string& host, int port, bool extra, int timeout) {
  return std::unique_ptr<client>( new client(id, host, port, extra, timeout, [](const std::string& msg) { py::print(msg); }));
}

PYBIND11_MODULE(_tws11, m) {

  m.doc() = "tws api";

  py::class_<Contract>(m, "contract")
    .def(py::init<>())
    .def_readwrite("id"      , &Contract::conId   )
    .def_readwrite("symbol"  , &Contract::symbol  )
    .def_readwrite("type"    , &Contract::secType )
    .def_readwrite("exchange", &Contract::exchange)
    .def_readwrite("currency", &Contract::currency)
    .def("__str__", [](const Contract& c) -> std::string {

        std::stringstream ss; ss
          << "contract id: " << c.conId << "\n"
          << "symbol:   " << c.symbol   << "\n"
          << "type:     " << c.secType  << "\n"
          << "exchange: " << c.exchange << "\n"
          << "currency: " << c.currency << "\n"
          ;

        return ss.str();
      }
    )
    ;

  py::class_<client>(m, "client")
    .def(py::init(&create_client)
      , py::arg("id")
      , py::arg("host")
      , py::arg("port")
      , py::arg("extra_auth") = false
      , py::arg("timeout") = 2000
    )
    .def("connect", [](client& cl, int timeout) -> bool {

        auto wait = clock_type::time_point::max();

        if (timeout > 0) {
          wait = clock_type::now() + std::chrono::seconds(timeout);
        }

        cl.connect();

        while(clock_type::now() < wait) {

          if (PyErr_CheckSignals() != 0) {        // handle ctrl + C
            throw py::error_already_set();
          }

          if (cl.connected()) {
            return true;
          }
        };

        return false;
      }
      , py::arg("timeout") = -1
    )
    .def("disconnect", &client::disconnect)
    .def("request", [](client& cl
      , Contract& c
      , const std::string& field
      , const std::string& bar
      , const std::string& dur
      , clock_type::time_point end
      , int timeout) -> py::dict {

        std::vector<clock_type::time_point> nanos;
        std::vector<double> high;
        std::vector<double> low;
        std::vector<double> open;
        std::vector<double> close;
        std::vector<double> wap;
        std::vector<std::int64_t> volume;
        std::vector<std::int64_t> count;

        cl.sink() = [&](TickerId id, const Bar& b) -> void {
          auto secs = std::chrono::seconds{ std::stoi( b.time ) };
           nanos.push_back(std::chrono::system_clock::time_point(secs));
            high.push_back(b.high   );
             low.push_back(b.low    );
            open.push_back(b.open   );
           close.push_back(b.close  );
             wap.push_back(b.wap    );
          volume.push_back(b.volume );
           count.push_back(b.count  );
        };

        try {

          auto wait = clock_type::time_point::max();

          if (timeout > 0) {
            wait = clock_type::now() + std::chrono::seconds(timeout);
          }
          
          cl.request(c, field, bar, dur, end);

          while(cl.run()) {

            if (clock_type::now() > wait) {                       // timeout
              break;
            }

            if (PyErr_CheckSignals() != 0) {        // handle ctrl + C
              cl.sink() = {};
              throw py::error_already_set();
            }          
          };

          cl.sink() = {};
          
        } catch(std::exception& ex) {
          py::print(ex.what());
          cl.sink() = {};
          throw ex;
        }

        return py::dict(
            "nanoseconds"_a = nanos
          ,        "high"_a = high
          ,         "low"_a = low
          ,        "open"_a = open
          ,       "close"_a = close
          ,         "wap"_a = wap
          ,      "volume"_a = volume
          ,       "count"_a = count
        );
      }
      , py::arg("contract")
      , py::arg("field")
      , py::arg("bar")
      , py::arg("duration")
      , py::arg("end")
      , py::arg("timeout") = -1
    )
    .def_property_readonly("id", &client::id)
    .def_property_readonly("host", &client::host)
    .def_property_readonly("port", &client::port)
    .def_property_readonly("extra_auth", &client::extra_auth)
    .def_property_readonly("connected", &client::connected)
    .def("__str__", [](const client& cl) -> std::string {

        std::ostringstream ss; ss 
          << "client host: '" << cl.host() << "'\n" 
          << "port:         " << cl.port() << "\n"
          ;
        return ss.str();
      }
    )
    ;
}