#include "infra/gdalalgo.h"
#include "infra/exception.h"

#include <gdal_alg.h>
#include <gdal_utils.h>

namespace inf::gdal {

VectorDataSet polygonize(const RasterDataSet& ds)
{
    auto memDriver = gdal::VectorDriver::create(gdal::VectorType::Memory);
    gdal::VectorDataSet memDataSet(memDriver.create_dataset("dummy"));
    auto layer = memDataSet.create_layer("Polygons");
    FieldDefinition def("Value", typeid(int32_t));
    layer.create_field(def);

    checkError(GDALPolygonize(ds.rasterband(1).get(), ds.rasterband(1).get(), layer.get(), 0, nullptr, nullptr, nullptr), "Failed to polygonize raster");
    return memDataSet;
}

class RasterizeOptionsWrapper
{
public:
    RasterizeOptionsWrapper(const std::vector<std::string>& opts)
    : _options(nullptr)
    {
        auto optionValues = create_options_array(opts);
        _options          = GDALRasterizeOptionsNew(const_cast<char**>(optionValues.data()), nullptr);
    }

    ~RasterizeOptionsWrapper()
    {
        GDALRasterizeOptionsFree(_options);
    }

    GDALRasterizeOptions* get()
    {
        return _options;
    }

private:
    GDALRasterizeOptions* _options;
};

template <typename T>
std::pair<GeoMetadata, std::vector<T>> rasterize(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options)
{
    RasterizeOptionsWrapper gdalOptions(options);

    std::vector<T> data(meta.rows * meta.cols);

    auto memDriver = gdal::RasterDriver::create(gdal::RasterType::Memory);
    gdal::RasterDataSet memDataSet(memDriver.create_dataset<T>(meta.rows, meta.cols, 0));
    memDataSet.add_band(data.data());
    memDataSet.set_geotransform(inf::metadata_to_geo_transform(meta));
    memDataSet.set_nodata_value(1, meta.nodata);
    memDataSet.set_projection(meta.projection);

    int errorCode = CE_None;
    GDALRasterize(nullptr, memDataSet.get(), ds.get(), gdalOptions.get(), &errorCode);
    if (errorCode != CE_None) {
        throw RuntimeError("Failed to rasterize dataset {}", errorCode);
    }

    return std::make_pair(read_metadata_from_dataset(memDataSet), std::move(data));
}

class VectorTranslateOptionsWrapper
{
public:
    VectorTranslateOptionsWrapper(const std::vector<std::string>& opts)
    : _options(nullptr)
    {
        auto optionValues = create_options_array(opts);
        _options          = GDALVectorTranslateOptionsNew(const_cast<char**>(optionValues.data()), nullptr);
    }

    ~VectorTranslateOptionsWrapper()
    {
        GDALVectorTranslateOptionsFree(_options);
    }

    GDALVectorTranslateOptions* get()
    {
        return _options;
    }

private:
    GDALVectorTranslateOptions* _options;
};

VectorDataSet translate_vector(const VectorDataSet& ds, const std::vector<std::string>& options)
{
    VectorTranslateOptionsWrapper gdalOptions(options);

    auto memDriver = gdal::VectorDriver::create(gdal::VectorType::Memory);
    gdal::VectorDataSet memDataSet(memDriver.create_dataset("dummy"));

    int errorCode              = CE_None;
    GDALDatasetH srcDataSetPtr = ds.get();
    GDALVectorTranslate(nullptr, memDataSet.get(), 1, &srcDataSetPtr, gdalOptions.get(), &errorCode);
    if (errorCode != CE_None) {
        throw RuntimeError("Failed to translate vector dataset {}", errorCode);
    }

    return memDataSet;
}

class WarpOptionsWrapper
{
public:
    WarpOptionsWrapper(const std::vector<std::string>& opts)
    : _options(nullptr)
    {
        auto optionValues = create_options_array(opts);
        _options          = GDALWarpAppOptionsNew(const_cast<char**>(optionValues.data()), nullptr);
    }

    ~WarpOptionsWrapper()
    {
        GDALWarpAppOptionsFree(_options);
    }

    GDALWarpAppOptions* get()
    {
        return _options;
    }

private:
    GDALWarpAppOptions* _options;
};

template <typename T>
std::pair<GeoMetadata, std::vector<T>> translate(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options)
{
    WarpOptionsWrapper gdalOptions(options);

    std::vector<T> data(meta.rows * meta.cols);

    auto memDriver = gdal::RasterDriver::create(gdal::RasterType::Memory);
    gdal::RasterDataSet memDataSet(memDriver.create_dataset<T>(meta.rows, meta.cols, 0));
    memDataSet.add_band(data.data());
    memDataSet.set_geotransform(inf::metadata_to_geo_transform(meta));
    memDataSet.set_nodata_value(1, meta.nodata);
    memDataSet.set_projection(meta.projection);

    int errorCode              = CE_None;
    GDALDatasetH srcDataSetPtr = ds.get();
    GDALWarp(nullptr, memDataSet.get(), 1, &srcDataSetPtr, gdalOptions.get(), &errorCode);
    if (errorCode != CE_None) {
        throw RuntimeError("Failed to translate dataset {}", errorCode);
    }

    return std::make_pair(read_metadata_from_dataset(memDataSet), std::move(data));
}

template std::pair<GeoMetadata, std::vector<float>> rasterize<float>(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<int32_t>> rasterize<int32_t>(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<uint8_t>> rasterize<uint8_t>(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);

template std::pair<GeoMetadata, std::vector<float>> translate<float>(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<int32_t>> translate<int32_t>(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<uint8_t>> translate<uint8_t>(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
}
