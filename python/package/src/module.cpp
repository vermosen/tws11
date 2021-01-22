
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

std::unique_ptr<client> create_client(int id, const std::string& host, int port, bool extra, int timeout, const std::string& tz) { // creator function to bind logger to py::print
  return std::unique_ptr<client>( new client(id, host, port, extra, timeout, [](const std::string& msg) { py::print(msg); }, tz));
}

PYBIND11_MODULE(_tws11, m) {

  m.doc() = "tws api";

  py::class_<Contract>(m, "contract")
    .def(py::init<>())
    .def_readwrite("id"        , &Contract::conId       )
    .def_readwrite("symbol"    , &Contract::symbol      )
    .def_readwrite("type"      , &Contract::secType     )
    .def_readwrite("exchange"  , &Contract::exchange    )
    .def_readwrite("currency"  , &Contract::currency    )
    .def_readwrite("multiplier", &Contract::multiplier  )
    .def_readwrite("side"      , &Contract::right       )
    .def_readwrite("strike"    , &Contract::strike      )
    .def_readwrite("expiry"    , &Contract::lastTradeDateOrContractMonth)
    .def_readwrite("category"  , &Contract::tradingClass)

    .def("__str__", [](const Contract& c) -> std::string {

        std::stringstream ss; ss
          << "contract id: " << c.conId       << "\n"
          << "symbol:     " << c.symbol       << "\n"
          << "type:       " << c.secType      << "\n"
          << "exchange:   " << c.exchange     << "\n"
          << "currency:   " << c.currency     << "\n"
          << "multiplier: " << c.multiplier   << "\n"
          << "side:       " << c.right        << "\n"
          << "strike:     " << c.strike       << "\n"
          << "expiry:     " << c.lastTradeDateOrContractMonth  << "\n" 
          << "category:   " << c.tradingClass << "\n"
          ;

        return ss.str();
      }
    )
    ;

  py::class_<ContractDetails>(m, "details")
    .def(py::init<>())
    .def_readwrite("contract", &ContractDetails::contract)
    ;

  py::class_<option_desc>(m, "option_description")
    .def(py::init<>())
    .def_readwrite("underlying"   , &option_desc::m_underlying)
    .def_readwrite("category"     , &option_desc::m_category)
    .def_readwrite("multiplier"   , &option_desc::m_multiplier)
    .def_readwrite("strike"       , &option_desc::m_strike)
    .def_readwrite("expiry"       , &option_desc::m_exp)
    .def_readwrite("exchange"     , &option_desc::m_exchange)
    .def("__str__", [](const option_desc& c) -> std::string {

        std::stringstream ss; ss
          << "underlying: " << c.m_underlying << "\n"
          << "category:   " << c.m_category   << "\n"
          << "multiplier: " << c.m_multiplier << "\n"
          << "strike:     " << c.m_strike     << "\n"
          << "expiry:     " << c.m_exp        << "\n"
          << "exchange:   " << c.m_exchange   << "\n"
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
      , py::arg("timezone") = "America/New_York"
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
        std::vector<double> high, low, open, close, wap;
        std::vector<std::int64_t> volume, count;

        cl.data_handle([&](TickerId id, const Bar& b) -> void {
          auto secs = std::chrono::seconds{ std::stoi( b.time ) };
           nanos.push_back(std::chrono::system_clock::time_point(secs));
            high.push_back(b.high   );
             low.push_back(b.low    );
            open.push_back(b.open   );
           close.push_back(b.close  );
             wap.push_back(b.wap    );
          volume.push_back(b.volume );
           count.push_back(b.count  );
        });

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

            if (PyErr_CheckSignals() != 0) {                      // handle ctrl + C
              cl.data_handle({});
              throw py::error_already_set();
            }          
          };

          cl.data_handle({});                                     // reset the sink
          
        } catch(const std::exception& ex) {
          py::print(ex.what());
          cl.data_handle({});
          throw ex;
        }

        return py::dict(
                   "time"_a = nanos
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
    .def("details", [](client& cl, Contract& c, int timeout) {

        std::vector<ContractDetails> retval;
        auto wait = clock_type::time_point::max();

        if (timeout > 0) {
          wait = clock_type::now() + std::chrono::seconds(timeout);
        }

        cl.details_handle([&](const ContractDetails& o) { retval.push_back(o); }); 
        
        try {
          cl.get_details(c);

          while(cl.run()) {

            if (clock_type::now() > wait) {                       // timeout
              break;
            }

            if (PyErr_CheckSignals() != 0) {                      // handle ctrl + C
              cl.data_handle({});
              throw py::error_already_set();
            }          
          };

          cl.details_handle({});                                  // reset the sink

        } catch(const std::exception& ex) {
          cl.details_handle({});
          throw ex;
        }

        return retval;
      }
    , py::arg("contract")
    , py::arg("timeout") = -1
    )
    .def("chain", [](client& cl, Contract& c, const std::string& exchange, int timeout) {

        // in stock chain requests, we need to query in bulk and filter 
        // the data based on the exchange which is very wasteful ...
        // see https://groups.io/g/twsapi/topic/68767782
        auto retval = std::vector<option_desc>();

        auto wait = clock_type::time_point::max();

        if (timeout > 0) {
          wait = clock_type::now() + std::chrono::seconds(timeout);
        }

        if (exchange == "") {
          cl.chain_handle([&](option_desc&& o) {
            retval.emplace_back(std::move(o));
          });
        } else {
          cl.chain_handle([&](option_desc&& o) {
            if (o.m_exchange == exchange) {
              retval.emplace_back(std::move(o));
            }
          });
        }

        try {
          if (c.secType == "STK") {
            cl.get_chain(c, "");
          }                                                       // TODO: future contracts would be handled differently

          while(cl.run()) {

            if (clock_type::now() > wait) {                       // timeout
              break;
            }

            if (PyErr_CheckSignals() != 0) {                      // handle ctrl + C
              cl.data_handle({});
              throw py::error_already_set();
            }          
          };

          cl.chain_handle({});                                    // reset the sink

        } catch(const std::exception& ex) {
          cl.chain_handle({});
          throw ex;
        }

        return retval;
      }
    , py::arg("contract")
    , py::arg("exchange") = ""
    , py::arg("timeout" ) = -1
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
          << "timezone:     " << cl.timezone() << "\n"
          ;
        return ss.str();
      }
    )
    ;
}