#include "arrow.h"

#include <vector>
#include <memory>
#include <sstream>

#include <arrow/type.h>
#include <arrow/config.h>
#include <arrow/status.h>
#include <arrow/table.h>

#include <arrow/api.h>

#include <arrow/python/pyarrow.h>

#include <pybind11/functional.h> // for nullptr conversion

// ref: https://github.com/vaexio/vaex-arrow-ext/blob/master/src/caster.hpp
namespace pybind11::detail {
     
    template <>
    struct type_caster<std::shared_ptr<arrow::Table>> {
    public:
      PYBIND11_TYPE_CASTER(std::shared_ptr<arrow::Table>, _("pyarrow::Table"));
      // Python -> C++
      bool load(handle src, bool) {
        PyObject* source = src.ptr();
        if (!arrow::py::is_array(source))
          return false;
        arrow::Result<std::shared_ptr<arrow::Table>> result = arrow::py::unwrap_table(source);
        if(!result.ok())
          return false;
        value = std::static_pointer_cast<arrow::Table>(result.ValueOrDie());
        return true;
      }
      // C++ -> Python
      static handle cast(std::shared_ptr<arrow::Table> src, return_value_policy /* policy */, handle /* parent */) {
        return arrow::py::wrap_table(src);
      }
    };
} // namespace pybind11::detail

namespace pybind11::detail {
    template <typename ArrayType> 
    struct gen_type_caster {
    public:
        // this doesn't work: PYBIND11_TYPE_CASTER(std::shared_ptr<ArrayType>, _(ArrayType::TypeClass::type_name()));
        PYBIND11_TYPE_CASTER(std::shared_ptr<ArrayType>, _("pyarrow::Array"));
        // Python -> C++
        bool load(handle src, bool) {
            PyObject* source = src.ptr();
            if (!arrow::py::is_array(source))
                return false;
            arrow::Result<std::shared_ptr<arrow::Array>> result = arrow::py::unwrap_array(source);
            if(!result.ok())
                return false;
            value = std::static_pointer_cast<ArrayType>(result.ValueOrDie());
            return true;
        }
        // C++ -> Python
        static handle cast(std::shared_ptr<ArrayType> src, return_value_policy /* policy */, handle /* parent */) {
            return arrow::py::wrap_array(src);
        }
    };
    template <>
    struct type_caster<std::shared_ptr<arrow::DoubleArray>> : public gen_type_caster<arrow::DoubleArray> {};
} // namespace pybind11::detail

namespace tws11_arrow {

  std::string info() {

    auto& info = arrow::GetBuildInfo();

    std::stringstream ss; ss
      << "arrow build summary:\n"
      << "version: " << info.version_string << "\n"
      << "shared lib version: " << info.so_version << "\n"
      << "compiler: " << info.compiler_id << " v." << info.compiler_version << "\n"
      << "flags: " << info.compiler_flags << "\n"
      << "git commit: " << info.git_id << "\n"
      << "package: " << info.package_kind << "\n"
      ;

    return ss.str();
  }

  double sum(std::shared_ptr<arrow::DoubleArray> a) {
    double sum = 0;
    for(int i = 0; i < a->length(); i++) {
        sum += a->Value(i);
    }
    return sum;
  }

  arrow::Status bar_func(std::shared_ptr<arrow::Table> table = nullptr) {

    /*
     * goal: replicate the following data structure
     *    return py::dict(
     *       "time"_a = nanos
     *  ,    "high"_a = high
     *  ,     "low"_a = low
     *  ,    "open"_a = open
     *  ,   "close"_a = close
     *  ,     "wap"_a = wap
     *  ,  "volume"_a = volume
     *  ,   "count"_a = count
     * );
     */

    auto* pool = arrow::default_memory_pool();

    arrow::TimestampBuilder time_builder(arrow::timestamp(arrow::TimeUnit::NANO), pool);

    arrow::DoubleBuilder    high_builder(pool);
    arrow::DoubleBuilder     low_builder(pool);
    arrow::DoubleBuilder    open_builder(pool);
    arrow::DoubleBuilder   close_builder(pool);
    arrow::DoubleBuilder     wap_builder(pool);
    arrow::Int64Builder   volume_builder(pool);
    arrow::Int64Builder    count_builder(pool);

    struct row_type {
      std::int64_t time;
      double high;
      double low;
      double open;
      double close;
      double wap;
      std::int64_t volume;
      std::int64_t count;
    };

    std::vector<row_type> rows = { { 1, 100.0, 99.0, 99.0, 100.0, 100.0, -1, -1 } };

    for (const auto& row : rows) {
      ARROW_RETURN_NOT_OK(  time_builder.Append(  row.time));
      ARROW_RETURN_NOT_OK(  high_builder.Append(  row.high));
      ARROW_RETURN_NOT_OK(   low_builder.Append(   row.low));
      ARROW_RETURN_NOT_OK(  open_builder.Append(  row.open));
      ARROW_RETURN_NOT_OK( close_builder.Append( row.close));
      ARROW_RETURN_NOT_OK(   wap_builder.Append(   row.wap));
      ARROW_RETURN_NOT_OK(volume_builder.Append(row.volume));
      ARROW_RETURN_NOT_OK( count_builder.Append( row.count));
    }
    
    std::shared_ptr<arrow::Array> id;
    
    std::vector<std::shared_ptr<arrow::Field>> fields = {
        arrow::field("time"  , arrow::timestamp(arrow::TimeUnit::NANO, "UTC"))
      , arrow::field("high"  , arrow::float64())
      , arrow::field("low"   , arrow::float64())
      , arrow::field("open"  , arrow::float64())
      , arrow::field("close" , arrow::float64())
      , arrow::field("wap"   , arrow::float64())
      , arrow::field("volume",   arrow::int64())
      , arrow::field("count" ,   arrow::int64())
    };

    std::shared_ptr<arrow::Array>   time_array; ARROW_RETURN_NOT_OK(time_builder.Finish(  &time_array));
    std::shared_ptr<arrow::Array>   high_array; ARROW_RETURN_NOT_OK(time_builder.Finish(  &high_array));
    std::shared_ptr<arrow::Array>    low_array; ARROW_RETURN_NOT_OK(time_builder.Finish(   &low_array));
    std::shared_ptr<arrow::Array>   open_array; ARROW_RETURN_NOT_OK(time_builder.Finish(  &open_array));
    std::shared_ptr<arrow::Array>  close_array; ARROW_RETURN_NOT_OK(time_builder.Finish( &close_array));
    std::shared_ptr<arrow::Array>    wap_array; ARROW_RETURN_NOT_OK(time_builder.Finish(   &wap_array));
    std::shared_ptr<arrow::Array> volume_array; ARROW_RETURN_NOT_OK(time_builder.Finish(&volume_array));
    std::shared_ptr<arrow::Array>  count_array; ARROW_RETURN_NOT_OK(time_builder.Finish( &count_array));
  
    auto schema = std::make_shared<arrow::Schema>(fields);

    table = arrow::Table::Make(schema, {
        time_array 
      , high_array
      , low_array
      , open_array
      , close_array
      , wap_array
      , volume_array
      , count_array
    });

    return arrow::Status::OK();
  }
}

namespace py = pybind11;
using namespace pybind11::literals;

void init_arrow_submodule(pybind11::module& m) {

  auto sub = m.def_submodule("_arrow")
    ;

  sub.doc() = "tws11 extension to use arrow tables";

/*   py::class_<arrow::Table, std::shared_ptr<arrow::Table>>(sub, "table")
    ; */

  py::class_<arrow::Status>(sub, "status")
    .def("__str__", &arrow::Status::ToString)
    ;

  sub.def("info", &tws11_arrow::info)
    ;

  sub.def("sum", &tws11_arrow::sum)
    ;

  sub.def("bar_func", [](std::shared_ptr<arrow::Table> ptr) {
        return tws11_arrow::bar_func(ptr);
      }
      , py::call_guard<py::gil_scoped_release>()
      , py::arg("table")
    )
    ;
}