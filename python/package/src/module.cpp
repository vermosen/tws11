
#include <sstream>

#include <pybind11/pybind11.h>

#include "include/tws.h"

PYBIND11_MODULE(_tws11, m) {

  m.doc() = "tws api";

  m.def("test", [](const std::string& msg) {

      std::stringstream ss; ss
        << "message: "
        << msg
        << std::endl;

      return ss.str();
    }
    , pybind11::arg("msg")
  );

  pybind11::class_<Contract>(m, "contract")
    .def(pybind11::init<>())
    .def_readwrite("id"      , &Contract::conId   )
    .def_readwrite("symbol"  , &Contract::symbol  )
    .def_readwrite("type"    , &Contract::secType )
    .def_readwrite("exchange", &Contract::exchange)
    .def_readwrite("currency", &Contract::currency)
    .def("__str__", [](const Contract& c) -> std::string {

        std::stringstream ss; ss
          << "contract id: " << c.conId << '\n'
          << "symbol:   " << c.symbol   << '\n'
          << "type:     " << c.secType  << '\n'
          << "exchange: " << c.exchange << '\n'
          << "currency: " << c.currency << '\n'
          ;

        return ss.str();
      }
    )
    ;
}