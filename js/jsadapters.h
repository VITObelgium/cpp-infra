#pragma once

#include "util/raster.h"
#include "util/rasterio.h"
#include "util/rastermetadata.h"

namespace gdx::js {

enum class DataType
{
    Byte,
    Int16,
    Uint16,
    Int32,
    Uint32,
    Float,
    Double
};

const std::type_info& typeInfoFromDataType(DataType dt);

Raster read_raster(const std::string& filepath);
Raster read_rasterAs(DataType dtype, const std::string& filepath);
Raster read_rasterFromMemory(const std::string& data);
Raster read_rasterFromMemoryAs(DataType dtype, const std::string& data);
void write_raster(const Raster& raster, const std::string& filepath);
void write_rasterColorMapped(const Raster& raster, const std::string& filepath, const std::string& cmapName);

Raster normaliseToByte(const Raster& raster);
Raster warp_raster(const Raster& raster, int32_t epsg);
Raster clusterSize(const Raster& raster, bool includeDiagonal);
}
