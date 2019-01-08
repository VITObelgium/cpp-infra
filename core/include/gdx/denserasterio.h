#pragma once

#include "gdx/denseraster.h"
#include "gdx/rasterio.h"
#include "infra/cast.h"
#include "infra/filesystem.h"
#include "infra/gdalalgo.h"

namespace gdx {

template <typename T>
DenseRaster<T> read_dense_raster(const fs::path& filename)
{
    auto dataSet = inf::gdal::RasterDataSet::create(filename);
    DenseRaster<T> raster(dataSet.y_size(), dataSet.x_size());
    raster.set_metadata(io::read_raster_data<T>(dataSet, raster));
    raster.init_nodata_values();
    return raster;
}

template <typename T>
DenseRaster<T> read_dense_raster(const fs::path& filename, const RasterMetadata& extent)
{
    auto dataSet = inf::gdal::RasterDataSet::create(filename);
    DenseRaster<T> raster(extent);
    raster.set_metadata(io::read_raster_data<T>(dataSet, extent, raster));
    raster.init_nodata_values();
    return raster;
}

template <typename T>
const RasterMetadata& read_raster(const fs::path& filename, DenseRaster<T>& raster)
{
    raster = read_dense_raster<T>(filename);
    return raster.metadata();
}

template <typename T>
const RasterMetadata& read_raster(const fs::path& filename, const RasterMetadata& extent, DenseRaster<T>& raster)
{
    raster = read_dense_raster<T>(filename, extent);
    return raster.metadata();
}

template <typename T>
void write_raster(DenseRaster<T>& raster, const fs::path& filename)
{
    raster.collapse_data();
    io::write_raster(raster, raster.metadata(), filename);
}

template <typename T>
void write_raster(DenseRaster<T>&& raster, const fs::path& filename)
{
    raster.collapse_data();
    io::write_raster(raster, raster.metadata(), filename);
}

template <typename T>
DenseRaster<T> warp_raster(const DenseRaster<T>& raster, int32_t destCrs)
{
    auto srcMeta  = raster.metadata();
    auto destMeta = inf::gdal::warp_metadata(raster.metadata(), destCrs);

    if (DenseRaster<T>::raster_type_has_nan) {
        // Set set source nodata to nan to avoid having to collapse the data
        // since the nodata values for floats are nan and not the actual nodata value
        srcMeta.nodata = DenseRaster<T>::NaN;
    }

    DenseRaster<T> result(destMeta, inf::truncate<T>(*destMeta.nodata));
    io::warp_raster<T, T>(raster, srcMeta, result, result.metadata());
    return result;
}

template <typename T>
DenseRaster<T> resample_raster(DenseRaster<T>& raster, const RasterMetadata& meta, inf::gdal::ResampleAlgorithm algo)
{
    const auto& srcMeta = raster.metadata();

    auto destMeta   = meta;
    destMeta.nodata = srcMeta.nodata;
    if (srcMeta.projection_epsg().has_value()) {
        destMeta.set_projection_from_epsg(srcMeta.projection_epsg().value());
    }

    DenseRaster<T> result(destMeta, inf::truncate<T>(*destMeta.nodata));
    io::warp_raster<T, T>(raster, raster.metadata(), result, result.metadata(), algo);
    result.init_nodata_values();
    return result;
}

}
