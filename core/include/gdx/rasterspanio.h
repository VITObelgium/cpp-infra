#pragma once

#include "gdx/rasterio.h"
#include "gdx/rastermetadata.h"
#include "gdx/rasterspan.h"

#include <vector>

namespace gdx {

template <typename T>
RasterMetadata read_raster(const fs::path& filename, std::vector<T>& raster)
{
    auto dataSet = inf::gdal::RasterDataSet::create(filename);
    raster.resize(dataSet.x_size() * dataSet.y_size());
    return io::read_raster_data<T>(dataSet, raster);
}

template <typename T>
RasterMetadata read_raster(const fs::path& filename, const RasterMetadata& extent, std::vector<T>& raster)
{
    auto dataSet = inf::gdal::RasterDataSet::create(filename);
    raster.resize(extent.rows * extent.cols);
    return io::read_raster_data<T>(dataSet, extent, raster);
}

template <typename T>
void write_raster(raster_span<const T> rasterData, const fs::path& filename)
{
    io::write_raster(rasterData, rasterData.metadata(), filename);
}

template <typename T>
void write_rasterColorMapped(raster_span<const T> rasterData, const fs::path& filename, const inf::ColorMap& cm)
{
    io::write_raster_color_mapped(rasterData, rasterData.metadata(), filename, cm);
}

template <typename T>
void write_raster(raster_span<const T> raster, const fs::path& filename, const std::type_info& type)
{
    io::write_raster(raster, raster.metadata(), filename, type);
}

}
