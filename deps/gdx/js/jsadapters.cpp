#include "jsadapters.h"
#include "util/log.h"
#include "util/maskedrasterio.h"
#include "util/rasterio.h"

#include "algo/clustersize.h"
#include "algo/normalise.h"

#include <fmt/format.h>
#include <gsl/span>
#include <sstream>

namespace gdx::js {

using cmap = infra::Cmap;

namespace {

template <typename ReturnType, typename Callable>
ReturnType safeOperation(Callable&& func)
{
    try {
        return func();
    } catch (const std::exception& e) {
        Log::error(e.what());
        throw;
    }
}
}

const std::type_info& typeInfoFromDataType(DataType dt)
{
    switch (dt) {
    case DataType::Byte: return typeid(uint8_t);
    case DataType::Int16: return typeid(int16_t);
    case DataType::Uint16: return typeid(uint16_t);
    case DataType::Int32: return typeid(int32_t);
    case DataType::Uint32: return typeid(uint32_t);
    case DataType::Float: return typeid(float);
    case DataType::Double: return typeid(double);
    }
}

template <typename Callable>
auto loggedCall(Callable&& c)
{
    try {
        return c();
    } catch (const std::exception& e) {
        Log::error(e.what());
        throw;
    }
}

gdx::Raster readRaster(const std::string& filepath)
{
    return loggedCall([&] { return Raster::read(filepath); });
}

gdx::Raster readRasterAs(DataType dtype, const std::string& filepath)
{
    return loggedCall([&] { return Raster::read(filepath, typeInfoFromDataType(dtype)); });
}

Raster readRasterFromMemory(const std::string& data)
{
    return loggedCall([&] { return Raster::readFromMemory("/vsimem/tempbuffer", gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(data.data()), data.size())); });
}

Raster readRasterFromMemoryAs(DataType dtype, const std::string& data)
{
    return loggedCall([&] { return Raster::readFromMemory("/vsimem/tempbufferas", gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(data.data()), data.size()), typeInfoFromDataType(dtype)); });
}

void writeRaster(const Raster& raster, const std::string& filepath)
{
    loggedCall([&] { raster.write(filepath, typeid(uint8_t)); });
}

void writeRasterColorMapped(const Raster& raster, const std::string& filepath, const std::string& cmapName)
{
    static const std::unordered_map<std::string, const infra::ColorDict&> cmapLookup1 = {
        {"summer", cmap::summer},
        {"autumn", cmap::autumn},
        {"bone", cmap::bone},
        {"cool", cmap::cool},
        {"copper", cmap::copper},
        {"gist_earth", cmap::gistEarth},
        {"gist_ncar", cmap::gistNcar},
        {"gist_stern", cmap::gistStern}};

    static const std::unordered_map<std::string, const std::vector<infra::Color>&> cmapLookup2 = {
        {"blues", cmap::Blues}};

    static const std::unordered_map<std::string, const std::vector<infra::ColorInfo>&> cmapLookup3 = {
        {"terrain", cmap::terrain}};

    static const std::unordered_map<std::string, const std::vector<infra::Color>&> cmapLookup4 = {
        {"pastel1", cmap::Pastel1},
        {"pastel2", cmap::Pastel2},
        {"set1", cmap::Set1},
        {"set2", cmap::Set2},
        {"set3", cmap::Set3},
        {"tab10", cmap::Tab10},
        {"tab20", cmap::Tab20},
        {"tab20b", cmap::Tab20b},
        {"tab20c", cmap::Tab20c}};

    loggedCall([&] {
        if (cmapLookup1.find(cmapName) != cmapLookup1.end()) {
            raster.writeColorMapped(filepath, infra::ColorMap(cmapLookup1.at(cmapName)));
        } else if (cmapLookup2.find(cmapName) != cmapLookup2.end()) {
            raster.writeColorMapped(filepath, infra::ColorMap(cmapLookup2.at(cmapName)));
        } else if (cmapLookup3.find(cmapName) != cmapLookup3.end()) {
            raster.writeColorMapped(filepath, infra::ColorMap(cmapLookup3.at(cmapName)));
        } else if (cmapLookup4.find(cmapName) != cmapLookup4.end()) {
            raster.writeColorMapped(filepath, infra::ColorMap::qualitative(cmapLookup4.at(cmapName)));
        } else {
            throw InvalidArgument("Unsupported color map: {}", cmapName);
        }
    });
}

Raster normaliseToByte(const Raster& raster)
{
    return safeOperation<Raster>([&]() {
        return std::visit([](auto&& ras) {
            MaskedRaster<uint8_t> normalisedData(ras.metaData());
            normalise(ras, normalisedData);
            return Raster(std::move(normalisedData));
        },
            raster.get());
    });
}

Raster warpRaster(const Raster& raster, int32_t epsg)
{
    return safeOperation<Raster>([&]() {
        return std::visit([epsg](auto&& ras) {
            return Raster(gdx::warpRaster(ras, epsg));
        },
            raster.get());
    });
}

Raster clusterSize(const Raster& raster, bool includeDiagonal)
{
    auto diagonalSetting = includeDiagonal ? ClusterDiagonals::Include : ClusterDiagonals::Exclude;
    return safeOperation<Raster>([&]() {
        return std::visit([diagonalSetting](auto&& ras) {
            return Raster(gdx::clusterSize(ras, diagonalSetting));
        },
            raster.get());
    });
}
}
