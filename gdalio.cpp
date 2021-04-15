#include "infra/gdalio.h"
#include "infra/cast.h"
#include "infra/exception.h"
#include "infra/filesystem.h"
#include "infra/string.h"

#include <array>
#include <cassert>
#include <complex>
#include <fstream>
#include <memory>
#include <typeindex>

namespace inf::gdal::io {

const std::type_info& get_raster_type(const fs::path& fileName)
{
    auto& type = gdal::RasterDataSet::open(fileName).band_datatype(1);
    if (type == typeid(void)) {
        throw RuntimeError("Unsupported raster data type");
    }

    return type;
}

GeoMetadata read_metadata(const fs::path& fileName)
{
    return gdal::RasterDataSet::open(fileName).geometadata();
}

void detail::read_raster_data(int bandNr, CutOut cut, const gdal::RasterDataSet& dataSet, const std::type_info& typeInfo, void* data, int dataCols)
{
    assert(bandNr > 0);

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

    dataSet.read_rasterdata(bandNr, std::max(0, cut.srcColOffset), std::max(0, cut.srcRowOffset), cut.cols, cut.rows, typeInfo, data, cut.cols, cut.rows, 0, truncate<int>(dataCols * typeSize));
}

void detail::create_output_directory_if_needed(const fs::path& p)
{
    if (str::starts_with(p.generic_u8string(), "/vsi")) {
        // this is a gdal virtual filesystem path
        return;
    }

    if (p.has_parent_path()) {
        inf::file::create_directory_if_not_exists(p.parent_path());
    }
}
}
