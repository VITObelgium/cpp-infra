#pragma once

#include "gdx/exception.h"
#include "infra/filesystem.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace gdx {

const std::type_info& dtypeToRasterType(pybind11::dtype type);
pybind11::dtype rasterTypeToDtype(const std::type_info& type);

template <typename Callable>
auto visitArray(const pybind11::array& arr, Callable&& visitor)
{
    auto& type = dtypeToRasterType(arr.dtype());
    if (type == typeid(uint8_t)) return visitor(arr.cast<pybind11::array_t<uint8_t>>());
    if (type == typeid(int16_t)) return visitor(arr.cast<pybind11::array_t<int16_t>>());
    if (type == typeid(uint16_t)) return visitor(arr.cast<pybind11::array_t<uint16_t>>());
    if (type == typeid(int32_t)) return visitor(arr.cast<pybind11::array_t<int32_t>>());
    if (type == typeid(uint32_t)) return visitor(arr.cast<pybind11::array_t<uint32_t>>());
    if (type == typeid(float)) return visitor(arr.cast<pybind11::array_t<float>>());
    if (type == typeid(double)) return visitor(arr.cast<pybind11::array_t<double>>());

    throw InvalidArgument("Invalid array data type");
}

fs::path handle_path(pybind11::object arg);

}
