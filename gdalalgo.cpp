#include "infra/gdalalgo.h"
#include "infra/exception.h"

#include <gdal_alg.h>
#include <gdal_utils.h>

namespace infra::gdal {

DataSet polygonize(const DataSet& ds)
{
    auto memDriver = gdal::VectorDriver::create(gdal::VectorType::Memory);
    gdal::DataSet memDataSet(memDriver.createDataSet("dummy"));
    auto layer = memDataSet.createLayer("Polygons");
    FieldDefinition def("Value", typeid(int32_t));
    layer.createField(def);

    checkError(GDALPolygonize(ds.rasterBand(1).get(), ds.rasterBand(1).get(), layer.get(), 0, nullptr, nullptr, nullptr), "Failed to polygonize raster");
    return memDataSet;
}

class RasterizeOptionsWrapper
{
public:
    RasterizeOptionsWrapper(const std::vector<std::string>& opts)
    : _options(nullptr)
    {
        auto optionValues = createOptionsArray(opts);
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
std::pair<GeoMetadata, std::vector<T>> rasterize(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options)
{
    RasterizeOptionsWrapper gdalOptions(options);

    std::vector<T> data(meta.rows * meta.cols);

    auto memDriver = gdal::RasterDriver::create(gdal::RasterType::Memory);
    gdal::DataSet memDataSet(memDriver.createDataSet<T>(meta.rows, meta.cols, 0, "dummy"));
    memDataSet.addBand(data.data());
    memDataSet.setGeoTransform(infra::metadataToGeoTransform(meta));
    memDataSet.setNoDataValue(1, meta.nodata);
    memDataSet.setProjection(meta.projection);

    int errorCode = CE_None;
    GDALRasterize(nullptr, memDataSet.get(), ds.get(), gdalOptions.get(), &errorCode);
    if (errorCode != CE_None) {
        throw RuntimeError("Failed to rasterize dataset {}", errorCode);
    }

    return std::make_pair(readMetadataFromDataset(memDataSet), std::move(data));
}

class VectorTranslateOptionsWrapper
{
public:
    VectorTranslateOptionsWrapper(const std::vector<std::string>& opts)
    : _options(nullptr)
    {
        auto optionValues = createOptionsArray(opts);
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

DataSet translateVector(const DataSet& ds, const std::vector<std::string>& options)
{
    VectorTranslateOptionsWrapper gdalOptions(options);

    auto memDriver = gdal::VectorDriver::create(gdal::VectorType::Memory);
    gdal::DataSet memDataSet(memDriver.createDataSet("dummy"));

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
        auto optionValues = createOptionsArray(opts);
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
std::pair<GeoMetadata, std::vector<T>> translate(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options)
{
    WarpOptionsWrapper gdalOptions(options);

    std::vector<T> data(meta.rows * meta.cols);

    auto memDriver = gdal::RasterDriver::create(gdal::RasterType::Memory);
    gdal::DataSet memDataSet(memDriver.createDataSet<T>(meta.rows, meta.cols, 0));
    memDataSet.addBand(data.data());
    memDataSet.setGeoTransform(infra::metadataToGeoTransform(meta));
    memDataSet.setNoDataValue(1, meta.nodata);
    memDataSet.setProjection(meta.projection);

    int errorCode              = CE_None;
    GDALDatasetH srcDataSetPtr = ds.get();
    GDALWarp(nullptr, memDataSet.get(), 1, &srcDataSetPtr, gdalOptions.get(), &errorCode);
    if (errorCode != CE_None) {
        throw RuntimeError("Failed to translate dataset {}", errorCode);
    }

    return std::make_pair(readMetadataFromDataset(memDataSet), std::move(data));
}

template std::pair<GeoMetadata, std::vector<float>> rasterize<float>(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<int32_t>> rasterize<int32_t>(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<uint8_t>> rasterize<uint8_t>(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);

template std::pair<GeoMetadata, std::vector<float>> translate<float>(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<int32_t>> translate<int32_t>(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<uint8_t>> translate<uint8_t>(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
}
