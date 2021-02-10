
#include <sstream>
#include <vector>
#include <unordered_set>

#include <tws/Contract.h>

#include "client.h"
#include "mtclient.h"

#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>
#include <pybind11/stl.h>

#include "arrow.h"

namespace py = pybind11;
using namespace pybind11::literals;

using clock_type = std::chrono::system_clock;

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
    .def(py::init([](int id, const std::string& host, int port, bool extra, int timeout, const std::string& tz) -> std::unique_ptr<client> {
        return std::unique_ptr<client>( 
          new client(id
            , host, port
            , extra, timeout
            , [](const std::string& msg) { py::print(msg); }
            , tz
        ));
      })
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
    /* .def("request", [](client& cl
      , const Contract& c
      , const std::string& field
      , const std::string& bar
      , const std::string& dur
      , clock_type::time_point end
      , int timeout) -> py::dict {

        std::vector<clock_type::time_point> nanos;
        std::vector<double> high, low, open, close, wap;
        std::vector<std::int64_t> volume, count;

        bool done = false;

        cl.completion_handle([&](int id) -> void { done = true; });

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
          
          cl.request(1, c, field, bar, dur, end);

          while(cl.run() && !done) {

            if (clock_type::now() > wait) {                       // timeout
              break;
            }

            if (PyErr_CheckSignals() != 0) {                      // handle ctrl + C
              cl.data_handle({});
              throw py::error_already_set();
            }            
          };

          cl.reset();                                             // reset the handles
          
        } catch(const std::exception& ex) {
          py::print(ex.what());
          cl.reset();
          throw ex;
        }

        return py::dict(
              "time"_a = nanos
          ,   "high"_a = high
          ,    "low"_a = low
          ,   "open"_a = open
          ,  "close"_a = close
          ,    "wap"_a = wap
          , "volume"_a = volume
          ,  "count"_a = count
        );
      }
      , py::arg("contract")
      , py::arg("field")
      , py::arg("bar")
      , py::arg("duration")
      , py::arg("end")
      , py::arg("timeout") = -1
    ) */
    .def("request", [](
        client& cl
      , const std::vector<Contract>& contracts
      , const std::string& field
      , const std::string& bar
      , const std::string& dur
      , clock_type::time_point end
      , int timeout
      , bool verbose) -> py::dict {

        std::vector<TickerId> ids;
        std::vector<clock_type::time_point> nanos;
        std::vector<double> high, low, open, close, wap;
        std::vector<std::int64_t> volume, count;

        std::size_t nrequest = contracts.size();
        std::size_t completed, failed;
        completed = 0; failed = 0;

        cl.completion_handle([&](int id) { 
          completed++; 
        });

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
             ids.push_back(id       );
        });

        auto wait = clock_type::time_point::max();

        if (timeout > 0) {
          wait = clock_type::now() + std::chrono::seconds(timeout);
        }
        
        try {                                                   // if an error is thrown while submitting -> abort

          std::size_t i = 0;
          for (auto& c : contracts) {
            cl.request(++i, c, field, bar, dur, end);
          }
        } catch(const std::exception& ex) {
          if (verbose) {
            std::stringstream ss; ss
              << "error submitting request: "
              << ex.what()
              ;

            py::print(ss.str());
          }

          cl.reset();
          throw ex;
        }
          
        while(completed + failed < nrequest) {

          try {                                                   // if an error is thrown while receiving -> continue until we recieved all the data

            if (!cl.run() || clock_type::now() > wait) {          // disconnected or timeout
              break;
            }

          } catch (const std::exception& ex) {

            failed++;                                             // we assume failures count here
            if (verbose) {
              std::stringstream ss; ss
                << "error while recieving data: "
                << ex.what()
                ;

              py::print(ss.str());
            }
          }
        };

        if (verbose) {
          std::stringstream ss; ss
            << "queried " 
            << nrequest
            << " contracts, retrieved: "
            << completed 
            << ", failed: "
            << failed
            ;

          py::print(ss.str());
        }
        
        return py::dict(
              "time"_a = nanos
          ,     "id"_a = ids
          ,   "high"_a = high
          ,    "low"_a = low
          ,   "open"_a = open
          ,  "close"_a = close
          ,    "wap"_a = wap
          , "volume"_a = volume
          ,  "count"_a = count
        );
      }
      , py::arg("contract")
      , py::arg("field")
      , py::arg("bar")
      , py::arg("duration")
      , py::arg("end")
      , py::arg("timeout") = -1
      , py::arg("verbose") = false
    )
    .def("details", [](
        client& cl
      , std::vector<Contract>& contracts
      , int timeout
      , bool verbose) {

        std::vector<ContractDetails> retval;
        std::unordered_set<int> ids;

        auto wait = clock_type::time_point::max();

        if (timeout > 0) {
          wait = clock_type::now() + std::chrono::seconds(timeout);
        }

        int failed, nrequest;
        failed = 0; nrequest = 0;

        cl.completion_handle([&](int id) { 
          ids.insert(id);
        });

        cl.details_handle(
          [&](int id, const ContractDetails& d) {                 // more than one key may be returned here
            retval.push_back(d);
          }
        ); 
        
        try {
          
          for (auto& c : contracts) {
            cl.get_details(++nrequest, c);
          }
          
        } catch(const std::exception& ex) {       // fatal
          py::print(ex.what());
          cl.reset();
          throw ex;
        }

        while(ids.size() + failed < nrequest) {

          try {                                                   // if an error is thrown while receiving -> continue until we recieved all the data

            if (!cl.run() || clock_type::now() > wait) {          // disconnected or timeout
              break;
            }

          } catch (const std::exception& ex) {

            failed++;                                             // we assume failures count here
            if (verbose) {
              std::stringstream ss; ss
                << "error while recieving data: "
                << ex.what()
                ;

              py::print(ss.str());
            }
          }
        };

        cl.reset();
        return retval;
      }
    , py::arg("contract")
    , py::arg("timeout") = -1
    , py::arg("verbose") = false
    )
    .def("chain", [](
        client& cl
      , Contract& c
      , const std::string& exchange
      , int timeout) {

        // in stock chain requests, we need to query in bulk and filter 
        // the data based on the exchange which is very wasteful ...
        // see https://groups.io/g/twsapi/topic/68767782
        auto retval = std::vector<option_desc>();

        auto wait = clock_type::time_point::max();

        if (timeout > 0) {
          wait = clock_type::now() + std::chrono::seconds(timeout);
        }

        bool completed = false;
        cl.completion_handle([&](int id) { completed = true; });

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

          while(cl.run() && !completed) {

            if (clock_type::now() > wait) {                       // timeout
              break;
            }

            if (PyErr_CheckSignals() != 0) {                      // handle ctrl + C
              throw py::error_already_set();
            }          
          };

          cl.reset();                                             // reset the sink

        } catch(const std::exception& ex) {
          cl.reset();
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

/* std::unique_ptr<mtclient> create_mtclient(int id, const std::string& host, int port, bool extra, int nthreads, int timeout, const std::string& tz) { // creator function to bind logger to py::print
  
  if (nthreads == -1) {
    nthreads = std::max(1U, std::thread::hardware_concurrency() - 1);
  }

  return std::unique_ptr<mtclient>( new mtclient(id, host, port, extra, nthreads, timeout, [](const std::string& msg) { py::print(msg); }, tz));
}

  py::class_<mtclient>(m, "mtclient")
    .def(py::init([](int id
      , const std::string& host
      , int port, bool extra
      , int nthreads, int timeout
      , const std::string& tz) -> std::unique_ptr<mtclient> {

        if (nthreads == -1) {
          nthreads = std::max(1U, std::thread::hardware_concurrency() - 1);
        }

        return std::unique_ptr<mtclient>(
          new mtclient(id, host, port, extra
            , nthreads, timeout
            , [](const std::string& msg) { py::print(msg); }, tz
          )
        );
      })
      , py::arg("id")
      , py::arg("host")
      , py::arg("port")
      , py::arg("extra_auth") = false
      , py::arg("nthreads") = -1
      , py::arg("timeout") = 2000
      , py::arg("timezone") = "America/New_York"
    )
    .def("connect", [](mtclient& cl, int timeout) -> bool {

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
    .def("disconnect", &mtclient::disconnect)
    .def("request", [](mtclient& cl
      , const std::vector<Contract>& contracts
      , const std::string& field
      , const std::string& bar
      , const std::string& dur
      , clock_type::time_point end
      , int timeout) -> py::dict {

        std::vector<clock_type::time_point> nanos;
        std::vector<double> high, low, open, close, wap;
        std::vector<std::int64_t> volume, count;

        try {

          auto wait = clock_type::time_point::max();

          if (timeout > 0) {
            wait = clock_type::now() + std::chrono::seconds(timeout);
          }
          
          // std::vector<future_type> futs;
          for (auto& c : contracts) {
            auto fut = cl.request_async(c, field, bar, dur, end);
          }

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
    ) */
    ;

  // init submodules
  init_arrow_submodule(m);
}