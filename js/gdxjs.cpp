#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "algo/clusterid.h"
#include "algo/clustersize.h"
#include "algo/conditionals.h"
#include "algo/conversion.h"
#include "algo/maximum.h"
#include "algo/minimum.h"
#include "algo/nodata.h"
#include "algo/reclass.h"
#include "algo/shape.h"
#include "algo/suminbuffer.h"
#include "infra/gdal.h"
#include "infra/point.h"
#include "util/exception.h"
#include "util/log.h"
#include "util/rasterio.h"

#include "jsadapters.h"
#include "util/raster.h"

#include <fmt/format.h>
#include <gsl/span>

namespace em = emscripten;

using Point  = gdx::Point<double>;
using Bounds = std::pair<Point, Point>;

static em::val getMetaNodata(gdx::RasterMetadata& meta)
{
    if (meta.nodata) {
        return em::val(*meta.nodata);
    }

    return em::val::undefined();
}

static em::val getRasterData(gdx::Raster& ras)
{
    return std::visit([](auto&& raster) {
        return em::val(em::typed_memory_view(raster.size(), raster.data()));
    },
        ras.get());
}

static void rasterSetProjection(gdx::Raster& ras, int32_t projection)
{
    ras.setProjection(projection);
}

static void rasterChangeNodata(gdx::Raster& ras, double nodata)
{
    ras.replaceNodata(nodata);
}

static void rasterChangeValue(gdx::Raster& ras, double oldValue, double newValue)
{
    ras.replaceValue(oldValue, newValue);
}

static void rasterFill(gdx::Raster& ras, double value)
{
    ras.fill(value);
}

static gdx::Raster createRaster(int32_t rows, int32_t cols, gdx::js::DataType type)
{
    return gdx::Raster(rows, cols, typeInfoFromDataType(type));
}

em::val rasterBounds(const gdx::RasterMetadata& meta)
{
    auto width  = meta.cols * meta.cellSize;
    auto height = meta.rows * meta.cellSize;

    auto minX = meta.xll;
    auto maxX = minX + width;
    auto minY = meta.yll;
    auto maxY = minY + height;

    auto topLeftCorner     = Point(minX, maxY);
    auto bottomRightCorner = Point(maxX, minY);

    if (auto epsg = meta.projectionEpsg(); epsg.has_value()) {
        topLeftCorner     = gdx::io::gdal::projectedToGeoGraphic(epsg.value(), topLeftCorner);
        bottomRightCorner = gdx::io::gdal::projectedToGeoGraphic(epsg.value(), bottomRightCorner);
    }

    std::swap(topLeftCorner.x, topLeftCorner.y);
    std::swap(bottomRightCorner.x, bottomRightCorner.y);
    return em::val(std::make_pair(topLeftCorner, bottomRightCorner));
}

EMSCRIPTEN_BINDINGS(gdx)
{
    gdx::Log::initialize("gdx");
    gdx::Log::warn("Gdx javascript bindings");

    gdx::io::gdal::registerGdal();

    em::enum_<gdx::Log::Level>("LogLevel")
        .value("Debug", gdx::Log::Level::Debug)
        .value("Info", gdx::Log::Level::Info)
        .value("Warning", gdx::Log::Level::Warning)
        .value("Error", gdx::Log::Level::Error)
        .value("Critical", gdx::Log::Level::Critical);

    em::enum_<gdx::js::DataType>("DataType")
        .value("Byte", gdx::js::DataType::Byte)
        .value("Int16", gdx::js::DataType::Int16)
        .value("Uint16", gdx::js::DataType::Uint16)
        .value("Int32", gdx::js::DataType::Int32)
        .value("Uint32", gdx::js::DataType::Uint32)
        .value("Float", gdx::js::DataType::Float)
        .value("Double", gdx::js::DataType::Double);

    em::value_array<Point>("Point")
        .element(&Point::x)
        .element(&Point::y);

    em::value_array<Bounds>("Bounds")
        .element(&Bounds::first)
        .element(&Bounds::second);

    em::class_<gdx::RasterMetadata>("RasterMetadata")
        .constructor()
        .constructor<int32_t, int32_t>()
        .function("setProjectionFromEpsg", &gdx::RasterMetadata::setProjectionFromEpsg)
        //.def_property_readonly("bounds_geo", [](const gdx::RasterMetadata& self) { return rasterBounds(self, false); })
        .property("rows", &gdx::RasterMetadata::rows)
        .property("cols", &gdx::RasterMetadata::cols)
        .property("cellSize", &gdx::RasterMetadata::cellSize)
        .property("xll", &gdx::RasterMetadata::xll)
        .property("yll", &gdx::RasterMetadata::yll)
        .function("nodata", &getMetaNodata)
        .property("projection", &gdx::RasterMetadata::projection)
        .function("bounds", &rasterBounds);

    em::class_<gdx::Raster>("Raster")
        .constructor(&createRaster)
        .function("metadata", &gdx::Raster::metaData)
        .function("data", &getRasterData)
        .function("setProjection", &rasterSetProjection)
        .function("changeNodata", &rasterChangeNodata)
        .function("changeValue", &rasterChangeValue)
        .function("fill", &rasterFill);

    em::function("set_log_level", &gdx::Log::setLevel);

    em::function("read", &gdx::js::readRaster);
    em::function("readAs", &gdx::js::readRasterAs);
    em::function("readFromMemory", &gdx::js::readRasterFromMemory);
    em::function("readFromMemoryAs", &gdx::js::readRasterFromMemoryAs);
    em::function("write", &gdx::js::writeRaster);
    em::function("writeColorMapped", &gdx::js::writeRasterColorMapped);
    em::function("warpRaster", &gdx::js::warpRaster);
    em::function("clusterSize", &gdx::js::clusterSize);
    em::function("normaliseToByte", &gdx::js::normaliseToByte);
}
