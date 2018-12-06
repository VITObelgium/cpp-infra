#include "infra/gdalalgo.h"
#include "infra/cast.h"
#include "infra/enumutils.h"
#include "infra/exception.h"

#include <gdal_alg.h>
#include <gdal_utils.h>

namespace inf::gdal {

void warp(const RasterDataSet& srcDataSet, RasterDataSet& dstDataSet, ResampleAlgorithm algo)
{
    static const std::array<const char*, 2> optionStrings{{"NUM_THREADS=ALL_CPUS", nullptr}};

    auto warpOptions              = GDALCreateWarpOptions();
    warpOptions->papszWarpOptions = CSLDuplicate(const_cast<char**>(optionStrings.data()));
    warpOptions->hSrcDS           = srcDataSet.get();
    warpOptions->hDstDS           = dstDataSet.get();
    warpOptions->nBandCount       = 1;
    warpOptions->panSrcBands      = reinterpret_cast<int*>(CPLMalloc(sizeof(int) * warpOptions->nBandCount));
    warpOptions->panSrcBands[0]   = 1;
    warpOptions->panDstBands      = reinterpret_cast<int*>(CPLMalloc(sizeof(int) * warpOptions->nBandCount));
    warpOptions->panDstBands[0]   = 1;
    warpOptions->pfnTransformer   = GDALGenImgProjTransform;
    warpOptions->eResampleAlg     = GDALResampleAlg(enum_value(algo));
    warpOptions->pTransformerArg  = gdal::checkPointer(GDALCreateGenImgProjTransformer(srcDataSet.get(),
                                                          nullptr,
                                                          dstDataSet.get(),
                                                          nullptr,
                                                          FALSE, 0.0, 0),
        "Failed to create actual warping transformer");

    GDALWarpOperation operation;
    operation.Initialize(warpOptions);
    checkError(operation.ChunkAndWarpImage(0, 0, dstDataSet.x_size(), dstDataSet.y_size()), "Failed to warp raster");

    GDALDestroyGenImgProjTransformer(warpOptions->pTransformerArg);
    GDALDestroyWarpOptions(warpOptions);
}

GeoMetadata warp_metadata(const GeoMetadata& meta, int32_t destCrs)
{
    if (meta.projection.empty()) {
        throw RuntimeError("Metadata does not contain projection information");
    }

    OGRSpatialReference destSpatialRef;
    if (destSpatialRef.importFromEPSG(destCrs) != OGRERR_NONE) {
        throw RuntimeError("Failed to set destination warp metadata projection");
    }

    char* destProjectionPtr = nullptr;
    destSpatialRef.exportToWkt(&destProjectionPtr);
    if (destProjectionPtr == nullptr) {
        throw RuntimeError("Failed to create destination warp metadata projection wkt");
    }

    GeoMetadata resultMeta;
    resultMeta.nodata     = meta.nodata;
    resultMeta.projection = destProjectionPtr;
    CPLFree(destProjectionPtr);

    auto memDriver  = gdal::RasterDriver::create(gdal::RasterType::Memory);
    auto srcDataSet = memDriver.create_dataset<uint8_t>(meta.rows, meta.cols, 0);
    srcDataSet.write_geometadata(meta);

    // Create a transformer that maps from source pixel/line coordinates
    // to destination georeferenced coordinates (not destination pixel line).
    // We do that by omitting the destination dataset handle (setting it to nullptr).
    auto* transformerArg = gdal::checkPointer(GDALCreateGenImgProjTransformer(srcDataSet.get(),
                                                  nullptr,
                                                  nullptr,
                                                  resultMeta.projection.c_str(),
                                                  FALSE, 0.0, 0),
        "Failed to create warping transformer");

    // Get information about the output size of the warped image
    std::array<double, 6> dstGeoTransform;
    gdal::checkError(GDALSuggestedWarpOutput(srcDataSet.get(), GDALGenImgProjTransform, transformerArg, dstGeoTransform.data(), &resultMeta.cols, &resultMeta.rows), "Failed to suggest warp output size");
    GDALDestroyGenImgProjTransformer(transformerArg);
    fill_geometadata_from_geo_transform(resultMeta, dstGeoTransform);

    return resultMeta;
}

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

    return std::make_pair(memDataSet.geometadata(), std::move(data));
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

    return std::make_pair(memDataSet.geometadata(), std::move(data));
}

template std::pair<GeoMetadata, std::vector<float>> rasterize<float>(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<int32_t>> rasterize<int32_t>(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<uint8_t>> rasterize<uint8_t>(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);

template std::pair<GeoMetadata, std::vector<float>> translate<float>(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<int32_t>> translate<int32_t>(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<uint8_t>> translate<uint8_t>(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
}
