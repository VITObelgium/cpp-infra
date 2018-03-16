#include "infra/gdalalgo.h"
#include "infra/exception.h"

#include <gdal_alg.h>
#include <gdal_utils.h>

namespace infra::gdal {

DataSet polygonize(const DataSet& ds)
{
    auto memDriver = gdal::Driver::create(gdal::VectorType::Memory);
    gdal::DataSet memDataSet(memDriver.createDataSet<int32_t>(0, 0, 0u, "dummy"));
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

    auto memDriver = gdal::Driver::create(gdal::VectorType::Memory);
    gdal::DataSet memDataSet(memDriver.createDataSet<T>(0, 0, 0u, "dummy"));
    memDataSet.addBand(data.data());
    memDataSet.setGeoTransform(infra::metadataToGeoTransform(meta));
    memDataSet.setNoDataValue(1, meta.nodata);
    memDataSet.setProjection(meta.projection);

    int errorCode = CE_None;
    auto result   = GDALRasterize(nullptr, memDataSet.get(), ds.get(), gdalOptions.get(), &errorCode);
    if (errorCode != CE_None) {
        throw RuntimeError("Failed to rasterize dataset {}", errorCode);
    }

    return std::make_pair(readMetadataFromDataset(memDataSet), std::move(data));
}

template std::pair<DataSet, std::vector<float>> rasterize<float>(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);

template std::pair<DataSet, std::vector<int32_t>> rasterize<int32_t>(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);

template std::pair<DataSet, std::vector<uint8_t>> rasterize<uint8_t>(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
}
