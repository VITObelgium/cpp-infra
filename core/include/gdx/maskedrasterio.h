#pragma once

#include "gdx/maskedraster.h"
#include "gdx/rasterio.h"
#include "infra/cast.h"
#include "infra/filesystem.h"
#include "infra/gdalalgo.h"

namespace gdx {

template <typename T>
MaskedRaster<T> read_masked_raster(const fs::path& filename)
{
    auto dataSet = inf::gdal::RasterDataSet::create(filename);
    MaskedRaster<T> raster(dataSet.y_size(), dataSet.x_size());
    raster.set_metadata(io::read_raster_data<T>(dataSet, raster));
    raster.init_nodata_values();
    return raster;
}

template <typename T>
MaskedRaster<T> read_masked_raster(const fs::path& filename, const RasterMetadata& extent)
{
    auto dataSet = inf::gdal::RasterDataSet::create(filename);
    MaskedRaster<T> raster(extent);
    raster.set_metadata(io::read_raster_data<T>(dataSet, extent, raster));
    raster.init_nodata_values();
    return raster;
}

template <typename T>
const RasterMetadata& read_raster(const fs::path& filename, MaskedRaster<T>& raster)
{
    raster = read_masked_raster<T>(filename);
    return raster.metadata();
}

template <typename T>
const RasterMetadata& read_raster(const fs::path& filename, const RasterMetadata& extent, MaskedRaster<T>& raster)
{
    raster = read_masked_raster<T>(filename, extent);
    return raster.metadata();
}

template <typename T>
void write_raster(const MaskedRaster<T>& raster, const fs::path& filename)
{
    const_cast<MaskedRaster<T>&>(raster).collapse_data();
    io::write_raster(raster, raster.metadata(), filename);
}

template <typename T>
void write_rasterColorMapped(const MaskedRaster<T>& raster, const fs::path& filename, const inf::ColorMap& cm)
{
    const_cast<MaskedRaster<T>&>(raster).collapse_data();
    io::write_raster_color_mapped(raster, raster.metadata(), filename, cm);
}

template <typename T>
void write_raster(const MaskedRaster<T>& raster, const fs::path& filename, const std::type_info& type)
{
    const_cast<MaskedRaster<T>&>(raster).collapse_data();
    io::write_raster(raster, raster.metadata(), filename, type);
}

template <typename T>
MaskedRaster<T> warp_raster(MaskedRaster<T>& raster, int32_t destCrs)
{
    auto destMeta = inf::gdal::warp_metadata(raster.metadata(), destCrs);
    MaskedRaster<T> result(destMeta, inf::truncate<T>(*destMeta.nodata));
    raster.collapse_data();
    io::warp_raster<T, T>(raster, raster.metadata(), result, result.metadata());
    result.init_nodata_values();
    return result;
}

template <typename T>
MaskedRaster<T> resample_raster(MaskedRaster<T>& raster, const RasterMetadata& meta, inf::gdal::ResampleAlgorithm algo)
{
    const auto& srcMeta = raster.metadata();

    auto destMeta   = meta;
    destMeta.nodata = srcMeta.nodata;
    if (srcMeta.projection_epsg().has_value()) {
        destMeta.set_projection_from_epsg(srcMeta.projection_epsg().value());
    }

    MaskedRaster<T> result(destMeta, inf::truncate<T>(destMeta.nodata.value_or(0.0)));
    io::warp_raster<T, T>(raster, raster.metadata(), result, result.metadata(), algo);
    result.init_nodata_values();
    return result;
}

}
