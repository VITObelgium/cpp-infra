#pragma once

#include "gdx/rastermetadata.h"
#include "infra/gdalalgo.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace gdx {

template <uint32_t>
struct RasterStats;

class Raster;

class cm
{
public:
    static const std::unordered_map<uint8_t, std::tuple<uint8_t, uint8_t, uint8_t>> ldd;
};

std::tuple<double, double> rowColCenterToXY(const gdx::RasterMetadata& meta, const std::tuple<int32_t, int32_t>& cell);
std::tuple<int32_t, int32_t> xYToRowCol(const gdx::RasterMetadata& meta, const std::tuple<double, double>& point);
bool isRowColOnRaster(const gdx::RasterMetadata& meta, const std::tuple<int32_t, int32_t>& cell);

pybind11::list rasterBounds(const gdx::RasterMetadata& meta, bool projected = true);

pybind11::object rasterNumpyMaskedArray(Raster& raster);
std::string rasterRepresentation(const gdx::Raster& raster);

Raster castRaster(const Raster& raster, pybind11::object dataType);
Raster read_raster(pybind11::object filepath);
Raster read_raster(pybind11::object filepath, const RasterMetadata& extent);
Raster read_raster(pybind11::object dataType, pybind11::object filepath);
void write_raster(Raster& raster, const std::string& filepath, pybind11::object colorMap);
void write_raster(pybind11::object dataType, Raster& raster, const std::string& filepath, pybind11::object colorMap);

Raster createFromNdArray(pybind11::array arrayData, const gdx::RasterMetadata& meta);

pybind11::str showMetadata(const RasterMetadata& raster);
pybind11::str showRasterStats(const RasterStats<512>& stats);
pybind11::object showRaster(pybind11::object rasterArg, pybind11::object colorMap, bool normalize);
pybind11::object showRaster(pybind11::array arr, const RasterMetadata& meta, pybind11::object colorMap, bool normalize);
void showRasterInBrowser(pybind11::object rasterArg, pybind11::object colorMap, bool normalize);
Raster warp_raster(pybind11::object rasterArg, int32_t epsg);
Raster resample(pybind11::object rasterArg, const RasterMetadata& meta, inf::gdal::ResampleAlgorithm algo);

bool any(pybind11::object rasterArg);
bool all(pybind11::object rasterArg);
}
