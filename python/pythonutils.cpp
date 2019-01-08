#include "pythonutils.h"

#include "gdx/exception.h"

namespace gdx {

namespace py = pybind11;
using namespace py::literals;

const std::type_info& dtypeToRasterType(py::dtype type)
{
    auto typeStr = static_cast<std::string>(py::str(type));

    if (typeStr == "bool") return typeid(uint8_t);
    if (typeStr == "uint8") return typeid(uint8_t);
    if (typeStr == "int16") return typeid(int16_t);
    if (typeStr == "uint16") return typeid(uint16_t);
    if (typeStr == "int32") return typeid(int32_t);
    if (typeStr == "uint32") return typeid(uint32_t);
    // when saying int as datatype it is translated to int64 on 64bit linux
    // assume the client simply wants a 32bit integer
    if (typeStr == "int64") return typeid(int32_t);
    if (typeStr == "uint64") return typeid(uint32_t);
    if (typeStr == "float32") return typeid(float);
    if (typeStr == "float64") return typeid(double);

    throw InvalidArgument("Unsupported numpy data type {}", typeStr);
}

py::dtype rasterTypeToDtype(const std::type_info& type)
{
    if (type == typeid(uint8_t)) return py::dtype::of<uint8_t>();
    if (type == typeid(int16_t)) return py::dtype::of<int16_t>();
    if (type == typeid(uint16_t)) return py::dtype::of<uint16_t>();
    if (type == typeid(int32_t)) return py::dtype::of<int32_t>();
    if (type == typeid(uint32_t)) return py::dtype::of<uint32_t>();
    if (type == typeid(float)) return py::dtype::of<float>();
    if (type == typeid(double)) return py::dtype::of<double>();

    throw InvalidArgument("Invalid raster data type");
}

fs::path handle_path(py::object arg)
{
    if (py::isinstance<py::str>(arg)) {
        return fs::u8path(std::string(py::str(arg)));
    } else {
        return fs::u8path(std::string(py::str(arg.attr("__str__")())));
    }
}
}
