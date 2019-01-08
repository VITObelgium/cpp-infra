#pragma once

#include "gdx/rastermetadata.h"
#include "infra/filesystem.h"
#include "infra/gdalalgo.h"

namespace gdx {

namespace gdal = inf::gdal;

template <typename T>
struct RasterizeOptions
{
    T burnValue     = 0;
    bool allTouched = false;
    inf::GeoMetadata meta;
    std::vector<std::string> values;
};

template <typename T>
struct TranslateOptions
{
    inf::GeoMetadata meta;
    std::vector<std::string> values;
};

template <typename RasterType>
RasterType rasterize(const inf::gdal::VectorDataSet& shapeDataSet, const RasterizeOptions<typename RasterType::value_type>& options)
{
    using T = typename RasterType::value_type;

    std::vector<std::string> gdalOpts;
    gdalOpts.push_back("-burn");
    gdalOpts.push_back(std::to_string(options.burnValue));

    if (options.allTouched) {
        gdalOpts.push_back("-at");
    }

    auto datasetDataPair = gdal::rasterize<T>(shapeDataSet, options.meta, gdalOpts);
    return RasterType(datasetDataPair.first, datasetDataPair.second);
}

template <typename RasterType>
RasterType rasterize(const fs::path& shapePath, const RasterizeOptions<typename RasterType::value_type>& options)
{
    return rasterize<RasterType>(gdal::VectorDataSet::create(shapePath), options);
}

template <template <typename> typename RasterType, typename T>
RasterType<T> translate(const RasterType<T>& ras, const TranslateOptions<T>& options)
{
    auto& meta = ras.metadata();

    auto memDriver = gdal::RasterDriver::create(gdal::RasterType::Memory);
    gdal::RasterDataSet memDataSet(memDriver.create_dataset<T>(meta.rows, meta.cols, 0));
    memDataSet.add_band(ras.data());
    memDataSet.set_geotransform(inf::metadata_to_geo_transform(meta));
    memDataSet.set_nodata_value(1, meta.nodata);
    memDataSet.set_projection(meta.projection);

    auto dataPair = gdal::translate<T>(memDataSet, options.meta);
    return RasterType<T>(dataPair.first, dataPair.second);
}
}
