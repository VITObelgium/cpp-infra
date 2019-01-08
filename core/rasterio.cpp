#include "gdx/rasterio.h"
#include "gdx/exception.h"
#include "gdxconfig.h"
#include "infra/cast.h"
#include "infra/filesystem.h"

#include <array>
#include <cassert>
#include <complex>
#include <fstream>
#include <memory>
#include <typeindex>

namespace gdx::io {

using namespace inf;

const std::type_info& get_raster_type(const fs::path& fileName)
{
    auto dataSet = gdal::RasterDataSet::create(fileName);
    switch (dataSet.band_datatype(1)) {
    case GDT_Byte: return typeid(uint8_t);
    case GDT_UInt16: return typeid(uint16_t);
    case GDT_Int16: return typeid(int16_t);
    case GDT_UInt32: return typeid(uint32_t);
    case GDT_Int32: return typeid(int32_t);
    case GDT_Float32: return typeid(float);
    case GDT_Float64: return typeid(double);
    // Map complex types to regular types so gdal will convert them
    case GDT_CInt16: return typeid(int16_t);
    case GDT_CInt32: return typeid(int32_t);
    case GDT_CFloat32: return typeid(float);
    case GDT_CFloat64: return typeid(double);
    case GDT_Unknown:
    default:
        throw RuntimeError("Unsupported raster data type");
    }
}

RasterMetadata read_metadata(const fs::path& fileName)
{
    return gdal::RasterDataSet::create(fileName).geometadata();
}

void detail::read_raster_data(CutOut cut, const gdal::RasterDataSet& dataSet, const std::type_info& typeInfo, void* data, int dataCols)
{
    static std::unordered_map<std::type_index, size_t> s_sizeLookup = {
        {std::type_index(typeid(float)), sizeof(float)},
        {std::type_index(typeid(double)), sizeof(double)},
        {std::type_index(typeid(uint8_t)), sizeof(uint8_t)},
        {std::type_index(typeid(uint16_t)), sizeof(uint16_t)},
        {std::type_index(typeid(uint32_t)), sizeof(uint32_t)},
        {std::type_index(typeid(int16_t)), sizeof(int16_t)},
        {std::type_index(typeid(int32_t)), sizeof(int32_t)}};

    auto typeSize = s_sizeLookup.at(std::type_index(typeInfo));

    std::byte* dataPtr = static_cast<std::byte*>(data);
    if (cut.dstRowOffset > 0) {
        dataPtr += (cut.dstRowOffset * dataCols) * typeSize;
    }

    if (cut.dstColOffset > 0) {
        dataPtr += cut.dstColOffset * typeSize;
    }

    data = dataPtr;

    dataSet.read_rasterdata(1, std::max(0, cut.srcColOffset), std::max(0, cut.srcRowOffset), cut.cols, cut.rows, typeInfo, data, cut.cols, cut.rows, 0, truncate<int>(dataCols * typeSize));
}
}
