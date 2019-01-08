#include "pythonioadapters.h"
#include "gdx/eigenio.h"
#include "gdx/rasterio.h"

#include "pythonutils.h"

#include <pybind11/eigen.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace pybind11 {
void flattenRaster(const array&)
{
}

const uint8_t* data(const array& a)
{
    return static_cast<const uint8_t*>(a.data());
}

ssize_t size(const array& a)
{
    return a.nbytes();
}
}

namespace gdx {

namespace py = pybind11;
using namespace py::literals;

template <typename T>
using EigenArray = Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

static RasterMetadata read_metadata(const fs::path& path)
{
    return io::read_metadata(path);
}

template <typename T>
static std::tuple<RasterMetadata, py::array> read_raster(const fs::path& path, const RasterMetadata* extent)
{
    auto rasterData = std::make_unique<EigenArray<T>>();
    auto meta       = read_raster<T>(path, *extent, *rasterData);
    return std::make_tuple<RasterMetadata, py::array>(std::move(meta), py::cast(rasterData.release(), py::return_value_policy::take_ownership));
}

py::object createNumpyMaskedArray(py::array data, py::array mask, std::optional<double> nodata)
{
    py::object maskedArray;
    auto ma = py::module::import("numpy.ma");
    if (mask) {
        maskedArray = ma.attr("array")(data, "mask"_a = mask, "fill_value"_a = nodata.value_or(0));
    } else {
        maskedArray = ma.attr("array")(data);
    }

    maskedArray.attr("_sharedmask") = false; // makes sure changes to the mask in python are done in the raster mask data
    return maskedArray;
}

template <typename T>
static std::tuple<RasterMetadata, py::object> read_masked_raster(const fs::path& path, const RasterMetadata* extent)
{
    auto rasterData = std::make_unique<EigenArray<T>>();
    auto meta       = read_raster<T>(path, *extent, *rasterData);
    py::array mask  = py::none();

    if (meta.nodata.has_value()) {
        auto mask_data = std::make_unique<EigenArray<bool>>(rasterData->rows(), rasterData->cols());
        std::transform(cbegin(*rasterData), cend(*rasterData), begin(*mask_data), [nodata = static_cast<T>(meta.nodata.value())](T value) {
            return value == nodata;
        });

        mask = py::cast(mask_data.release(), py::return_value_policy::take_ownership);
    }

    py::array data = py::cast(rasterData.release(), py::return_value_policy::take_ownership);
    auto nodata    = meta.nodata;

    return std::make_tuple<RasterMetadata, py::object>(std::move(meta), createNumpyMaskedArray(data, mask, nodata));
}

static std::tuple<RasterMetadata, py::array> readArray(py::object path, py::object dtype, const RasterMetadata* extent)
{
    auto fsPath = handle_path(path);

    const auto& type = [&]() -> const std::type_info& {
        if (dtype.is_none()) {
            return io::get_raster_type(fsPath);
        } else {
            return dtypeToRasterType(py::dtype::from_args(dtype));
        }
    }();

    if (type == typeid(uint8_t)) return read_raster<uint8_t>(fsPath, extent);
    if (type == typeid(int16_t)) return read_raster<int16_t>(fsPath, extent);
    if (type == typeid(uint16_t)) return read_raster<uint16_t>(fsPath, extent);
    if (type == typeid(int32_t)) return read_raster<int32_t>(fsPath, extent);
    if (type == typeid(uint32_t)) return read_raster<uint32_t>(fsPath, extent);
    if (type == typeid(float)) return read_raster<float>(fsPath, extent);
    if (type == typeid(double)) return read_raster<double>(fsPath, extent);

    throw RuntimeError("Unsupported raster data type");
}

static std::tuple<RasterMetadata, py::array> readMaskedArray(py::object path, py::object dtype, const RasterMetadata* extent)
{
    auto fsPath = handle_path(path);

    const auto& type = [&]() -> const std::type_info& {
        if (dtype.is_none()) {
            return io::get_raster_type(fsPath);
        } else {
            return dtypeToRasterType(py::dtype::from_args(dtype));
        }
    }();

    if (type == typeid(uint8_t)) return read_masked_raster<uint8_t>(fsPath, extent);
    if (type == typeid(int16_t)) return read_masked_raster<int16_t>(fsPath, extent);
    if (type == typeid(uint16_t)) return read_masked_raster<uint16_t>(fsPath, extent);
    if (type == typeid(int32_t)) return read_masked_raster<int32_t>(fsPath, extent);
    if (type == typeid(uint32_t)) return read_masked_raster<uint32_t>(fsPath, extent);
    if (type == typeid(float)) return read_masked_raster<float>(fsPath, extent);
    if (type == typeid(double)) return read_masked_raster<double>(fsPath, extent);

    throw RuntimeError("Unsupported raster data type");
}

static void writeArray(py::array data, const RasterMetadata& meta, py::object path, py::object dtype)
{
    const auto& type = [&]() -> const std::type_info& {
        if (dtype.is_none()) {
            return io::get_raster_type(handle_path(path));
        } else {
            return dtypeToRasterType(py::dtype::from_args(dtype));
        }
    }();

    if (!(data.flags() & py::detail::npy_api::NPY_ARRAY_C_CONTIGUOUS_)) {
        throw RuntimeError("Only contiguous arrays can be written to disk");
    }

    if (data.ndim() != 2) {
        throw RuntimeError("Only 2 dimenstional arrays can be written to disk");
    }

    if (meta.rows != data.shape()[0] ||
        meta.cols != data.shape()[1]) {
        throw RuntimeError("Metadata size does not match array size");
    }

    io::write_raster(data, meta, handle_path(path), type);
}

void initIoModule(py::module& ioMod)
{
    ioMod.def("read_metadata",
        &read_metadata,
        "raster_path"_a,
        "Read the metadata from a raster on disk, returns the gdx.raster_metadata instance");

    ioMod.def("read_array",
        &readArray,
        "raster_path"_a,
        "dtype"_a  = py::none(),
        "extent"_a = py::none(),
        "Read a raster from disk, returns a (gdx.raster_metadata, np.ndarray) tuple");

    ioMod.def("read_maskedarray",
        &readMaskedArray,
        "raster_path"_a,
        "dtype"_a  = py::none(),
        "extent"_a = py::none(),
        "Read a raster from disk, returns a (gdx.raster_metadata, np.ma.MaskedArray) tuple");

    ioMod.def("write_array",
        &writeArray,
        "data"_a,
        "metadata"_a,
        "raster_path"_a,
        "dtype"_a = py::none(),
        "Write a numpy array to disk");
}
}
