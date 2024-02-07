#include "infra/gdalalgo.h"
#include "infra/cast.h"
#include "infra/enumutils.h"
#include "infra/exception.h"
#include "infra/gdalio.h"
#include "infra/string.h"

#include <cassert>
#include <gdal_alg.h>
#include <gdal_utils.h>

namespace inf::gdal {

void warp(const RasterDataSet& srcDataSet, RasterDataSet& dstDataSet, ResampleAlgorithm algo)
{
    WarpOptions options;
    options.resampleAlgo = algo;
    return warp(srcDataSet, dstDataSet, options);
}

void warp(const RasterDataSet& srcDataSet, RasterDataSet& dstDataSet, WarpOptions& options)
{
    std::vector<std::string> strOptions = {
        "NUM_THREADS=ALL_CPUS",
    };

    strOptions.reserve(strOptions.size() + options.additionalOptions.size());
    std::copy(options.additionalOptions.begin(), options.additionalOptions.end(), std::back_inserter(strOptions));

    auto warpOptions              = GDALCreateWarpOptions();
    warpOptions->papszWarpOptions = CSLDuplicate(create_string_list(strOptions));
    warpOptions->hSrcDS           = srcDataSet.get();
    warpOptions->hDstDS           = dstDataSet.get();
    warpOptions->nBandCount       = 1;
    warpOptions->panSrcBands      = reinterpret_cast<int*>(CPLMalloc(sizeof(int) * warpOptions->nBandCount));
    warpOptions->panSrcBands[0]   = 1;
    warpOptions->panDstBands      = reinterpret_cast<int*>(CPLMalloc(sizeof(int) * warpOptions->nBandCount));
    warpOptions->panDstBands[0]   = 1;
    warpOptions->pfnTransformer   = GDALGenImgProjTransform;
    warpOptions->eResampleAlg     = GDALResampleAlg(enum_value(options.resampleAlgo));

    if (options.clipPolygon.get() != nullptr) {
        warpOptions->hCutline = options.clipPolygon.get()->clone();
    }

    if (options.clipBlendDistance.has_value()) {
        warpOptions->dfCutlineBlendDist = *options.clipBlendDistance;
    }

    auto srcNodataValue = srcDataSet.nodata_value(1);
    if (srcNodataValue.has_value()) {
        // will get freed by gdal
        warpOptions->padfSrcNoDataReal    = static_cast<double*>(CPLMalloc(warpOptions->nBandCount * sizeof(double)));
        warpOptions->padfSrcNoDataReal[0] = srcNodataValue.value();
    }

    auto dstNodataValue = dstDataSet.nodata_value(1);
    if (dstNodataValue.has_value()) {
        // will get freed by gdal
        warpOptions->padfDstNoDataReal    = static_cast<double*>(CPLMalloc(warpOptions->nBandCount * sizeof(double)));
        warpOptions->padfDstNoDataReal[0] = dstNodataValue.value();
    }

    warpOptions->pTransformerArg = gdal::check_pointer(GDALCreateGenImgProjTransformer(srcDataSet.get(),
                                                                                       nullptr,
                                                                                       dstDataSet.get(),
                                                                                       nullptr,
                                                                                       FALSE, 0.0, 0),
                                                       "Failed to create actual warping transformer");

    GDALWarpOperation operation;
    operation.Initialize(warpOptions);
    check_error(operation.ChunkAndWarpImage(0, 0, dstDataSet.x_size(), dstDataSet.y_size()), "Failed to warp raster");

    GDALDestroyGenImgProjTransformer(warpOptions->pTransformerArg);
    GDALDestroyWarpOptions(warpOptions);
}

class GdalWarpAppOptionsWrapper
{
public:
    GdalWarpAppOptionsWrapper() noexcept
    : _opts(GDALWarpAppOptionsNew(nullptr, nullptr))
    {
    }

    GdalWarpAppOptionsWrapper(const std::vector<std::string>& opts)
    : _opts(GDALWarpAppOptionsNew(create_string_list(opts), nullptr))
    {
        if (!_opts) {
            throw RuntimeError("Invalid warp arguments");
        }
    }

    ~GdalWarpAppOptionsWrapper() noexcept
    {
        GDALWarpAppOptionsFree(_opts);
    }

    operator GDALWarpAppOptions*() noexcept
    {
        return _opts;
    }

    void set_option(const char* key, const char* value)
    {
        GDALWarpAppOptionsSetWarpOption(_opts, key, value);
    }

private:
    GDALWarpAppOptions* _opts = nullptr;
};

void warp(const RasterDataSet& srcDataSet, RasterDataSet& dstDataSet, const std::vector<std::string>& options, const std::vector<std::pair<std::string, std::string>>& keyValueOptions)
{
    GdalWarpAppOptionsWrapper warpOptions(options);
    for (auto& [key, value] : keyValueOptions) {
        warpOptions.set_option(key.c_str(), value.c_str());
    }

    int usageError = 0;
    auto srcHandle = GDALDataset::ToHandle(srcDataSet.get());
    GDALWarp(nullptr, GDALDataset::ToHandle(dstDataSet.get()), 1, &srcHandle, warpOptions, &usageError);
    if (usageError) {
        throw RuntimeError("Warp failed");
    }
}

VectorDataSet warp_vector(const fs::path& vectorPath, const GeoMetadata& destMeta, const std::vector<std::string>& extraOptions)
{
    return warp(gdal::VectorDataSet::open(vectorPath), destMeta, extraOptions);
}

VectorDataSet warp_vector(const fs::path& vectorPath, const std::string& projection, const std::vector<std::string>& extraOptions)
{
    return warp(gdal::VectorDataSet::open(vectorPath), projection, extraOptions);
}

VectorDataSet warp(const VectorDataSet& srcDataSet, const std::string& projection, const std::vector<std::string>& extraOptions)
{
    std::vector<std::string> options = {
        "-t_srs"s,
        projection,
    };

    std::copy(extraOptions.begin(), extraOptions.end(), std::back_inserter(options));

    return translate_vector(srcDataSet, options);
}

VectorDataSet warp(const VectorDataSet& srcDataSet, const GeoMetadata& destMeta, const std::vector<std::string>& extraOptions)
{
    auto [xMin, yMax] = destMeta.top_left();
    auto [xMax, yMin] = destMeta.bottom_right();

    std::vector<std::string> options = {
        "-t_srs"s,
        destMeta.projection,
        "-clipdst",
        fmt::format("{:f}", xMin),
        fmt::format("{:f}", yMin),
        fmt::format("{:f}", xMax),
        fmt::format("{:f}", yMax),
    };

    std::copy(extraOptions.begin(), extraOptions.end(), std::back_inserter(options));

    return translate_vector(srcDataSet, options);
}

GeoMetadata warp_metadata(const GeoMetadata& meta, const std::string& destProjection)
{
    if (meta.projection.empty()) {
        throw RuntimeError("Metadata does not contain projection information");
    }

    GeoMetadata resultMeta;
    resultMeta.nodata     = meta.nodata;
    resultMeta.projection = destProjection;

    auto memDriver  = gdal::RasterDriver::create(gdal::RasterType::Memory);
    auto srcDataSet = memDriver.create_dataset<uint8_t>(meta.rows, meta.cols, 0);
    srcDataSet.write_geometadata(meta);

    // Create a transformer that maps from source pixel/line coordinates
    // to destination georeferenced coordinates (not destination pixel line).
    // We do that by omitting the destination dataset handle (setting it to nullptr).
    auto* transformerArg = gdal::check_pointer(GDALCreateGenImgProjTransformer(srcDataSet.get(),
                                                                               nullptr,
                                                                               nullptr,
                                                                               resultMeta.projection.c_str(),
                                                                               FALSE, 0.0, 0),
                                               "Failed to create warping transformer");

    // Get information about the output size of the warped image
    std::array<double, 6> dstGeoTransform;
    check_error(GDALSuggestedWarpOutput(srcDataSet.get(), GDALGenImgProjTransform, transformerArg, dstGeoTransform.data(), &resultMeta.cols, &resultMeta.rows), "Failed to suggest warp output size");
    GDALDestroyGenImgProjTransformer(transformerArg);
    fill_geometadata_from_geo_transform(resultMeta, dstGeoTransform);

    return resultMeta;
}

GeoMetadata warp_metadata(const GeoMetadata& meta, int32_t destCrs)
{
    return warp_metadata(meta, gdal::SpatialReference(destCrs).export_to_wkt());
}

VectorDataSet polygonize(const RasterDataSet& ds)
{
    auto memDriver = gdal::VectorDriver::create(gdal::VectorType::Memory);
    gdal::VectorDataSet memDataSet(memDriver.create_dataset("dummy"));
    auto layer = memDataSet.create_layer("Polygons");
    FieldDefinition def("Value", typeid(int32_t));
    layer.create_field(def);
    if (auto projection = ds.projection(); !projection.empty()) {
        SpatialReference srs(projection);
        layer.set_projection(srs);
    }

    check_error(GDALPolygonize(ds.rasterband(1).get(), ds.rasterband(1).get(), layer.handle(), 0, nullptr, nullptr, nullptr), "Failed to polygonize raster");
    return memDataSet;
}

class RasterizeOptionsWrapper
{
public:
    RasterizeOptionsWrapper(const std::vector<std::string>& opts)
    : _options(nullptr)
    {
        auto optionValues = create_string_list(opts);
        _options          = GDALRasterizeOptionsNew(optionValues.List(), nullptr);

        if (_options == nullptr && !opts.empty()) {
            gdal::throw_last_error("Invalid rasterize options provided");
        }
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

    std::vector<T> data(meta.rows * meta.cols, truncate<T>(meta.nodata.value_or(0.0)));

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

RasterDataSet rasterize_to_disk(const VectorDataSet& ds, const fs::path& path, const std::vector<std::string>& options)
{
    RasterizeOptionsWrapper gdalOptions(options);

    int errorCode = CE_None;
    RasterDataSet rasterDs(GDALRasterize(path.generic_u8string().c_str(), nullptr, ds.get(), gdalOptions.get(), &errorCode));
    if (errorCode != CE_None) {
        throw RuntimeError("Failed to rasterize dataset {}", errorCode);
    }

    return rasterDs;
}

class VectorTranslateOptionsWrapper
{
public:
    VectorTranslateOptionsWrapper(const std::vector<std::string>& opts)
    : _options(nullptr)
    {
        auto optionValues = create_string_list(opts);
        _options          = GDALVectorTranslateOptionsNew(optionValues.List(), nullptr);
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

VectorDataSet translate_vector_to_disk(const VectorDataSet& ds, const fs::path& path, const std::vector<std::string>& options)
{
    VectorTranslateOptionsWrapper gdalOptions(options);

    io::detail::create_output_directory_if_needed(path);

    int errorCode              = CE_None;
    GDALDatasetH srcDataSetPtr = ds.get();
    VectorDataSet resultDs(GDALDataset::FromHandle(GDALVectorTranslate(str::from_u8(path.u8string()).c_str(), nullptr, 1, &srcDataSetPtr, gdalOptions.get(), &errorCode)));
    if (errorCode != CE_None) {
        throw RuntimeError("Failed to translate vector dataset to disk {}", errorCode);
    }

    if (!resultDs.is_valid()) {
        throw RuntimeError("Failed to translate vector dataset to disk");
    }

    return resultDs;
}

VectorDataSet buffer_vector(VectorDataSet& ds, const BufferOptions opts)
{
    assert(opts.distance > 0.0);

    auto memDriver = VectorDriver::create(VectorType::Memory);
    auto memDs     = memDriver.create_dataset();

    for (int i = 0; i < ds.layer_count(); ++i) {
        auto srcLayer        = ds.layer(i);
        auto dstGeometryType = opts.geometryType.value_or(srcLayer.geometry_type());
        auto dstLayer        = memDs.create_layer(srcLayer.name(), dstGeometryType);

        // Take over the projection information
        if (auto srs = srcLayer.projection(); srs.has_value()) {
            dstLayer.set_projection(*srs);
        }

        // Setting all field indexes to -1 only copies the geometry
        std::vector<int> fieldIndexes(srcLayer.layer_definition().field_count(), -1);

        if (opts.includeFields) {
            // Take over the field definitions
            auto layerDef = srcLayer.layer_definition();
            for (int fieldIndex = 0; fieldIndex < layerDef.field_count(); ++fieldIndex) {
                gdal::FieldDefinition defCopy(layerDef.field_definition(fieldIndex));
                dstLayer.create_field(defCopy);
                fieldIndexes[fieldIndex] = dstLayer.field_index(defCopy.name());
            }
        }

        if (!opts.attributeFilter.empty()) {
            srcLayer.set_attribute_filter(opts.attributeFilter);
        }

        for (auto& feature : srcLayer) {
            if (feature.has_geometry()) {
                Feature feat(dstLayer.layer_definition());

                if (opts.includeFields) {
                    feat.set_fields_from(feature, Feature::FieldCopyMode::Strict, fieldIndexes);
                }

                feat.set_geometry(feature.geometry().buffer(opts.distance, opts.numQuadSegments));
                dstLayer.create_feature(feat);
            }
        }
    }

    return memDs;
}

class WarpOptionsWrapper
{
public:
    WarpOptionsWrapper(const std::vector<std::string>& opts)
    : _options(nullptr)
    {
        auto optionValues = create_string_list(opts);
        _options          = GDALWarpAppOptionsNew(optionValues.List(), nullptr);
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

class TranslateOptionsWrapper
{
public:
    TranslateOptionsWrapper(const std::vector<std::string>& opts)
    : _options(nullptr)
    {
        auto optionValues = gdal::create_string_list(opts);
        _options          = GDALTranslateOptionsNew(const_cast<char**>(optionValues.List()), nullptr);
    }

    TranslateOptionsWrapper(const std::vector<std::string>& opts, ProgressInfo& progress)
    : TranslateOptionsWrapper(opts)
    {
        if (progress.is_valid()) {
            GDALProgressFunc progressFunc = [](double complete, const char* /*message*/, void* progressArg) -> int {
                auto* cb = reinterpret_cast<ProgressInfo*>(progressArg);
                cb->tick(truncate<float>(complete));
                return cb->cancel_requested() ? FALSE : TRUE;
            };

            GDALTranslateOptionsSetProgress(_options, progressFunc, &progress);
        }
    }

    ~TranslateOptionsWrapper()
    {
        GDALTranslateOptionsFree(_options);
    }

    GDALTranslateOptions* get()
    {
        return _options;
    }

private:
    GDALTranslateOptions* _options;
};

gdal::RasterDataSet translate(const fs::path& inputPath, const std::vector<std::string>& options, const ProgressInfo::Callback& progressCb)
{
    return translate(inputPath, {}, options, progressCb);
}

gdal::RasterDataSet translate(const fs::path& inputPath, const fs::path& outputPath, const std::vector<std::string>& options, const ProgressInfo::Callback& progressCb)
{
    auto ds = gdal::RasterDataSet::open(inputPath);
    return translate(ds, outputPath, options, progressCb);
}

gdal::RasterDataSet translate(const gdal::RasterDataSet& ds, const std::vector<std::string>& options, const ProgressInfo::Callback& progressCb)
{
    return translate(ds, fs::path(), options, progressCb);
}

gdal::RasterDataSet translate(const gdal::RasterDataSet& ds, const fs::path& outputPath, const std::vector<std::string>& options, const ProgressInfo::Callback& progressCb)
{
    ProgressInfo progress(progressCb);
    TranslateOptionsWrapper gdalOptions(options, progress);

    int userError = 0;
    auto resultDs = gdal::RasterDataSet(GDALTranslate(outputPath.empty() ? nullptr : file::generic_u8string(outputPath).c_str(), ds.get(), gdalOptions.get(), &userError));

    if (userError) {
        throw RuntimeError("Translate: invalid arguments");
    }

    return resultDs;
}

template std::pair<GeoMetadata, std::vector<float>> rasterize<float>(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<double>> rasterize<double>(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<int32_t>> rasterize<int32_t>(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<int16_t>> rasterize<int16_t>(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<uint8_t>> rasterize<uint8_t>(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);

template std::pair<GeoMetadata, std::vector<float>> translate<float>(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<double>> translate<double>(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<int32_t>> translate<int32_t>(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
template std::pair<GeoMetadata, std::vector<uint8_t>> translate<uint8_t>(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options);
}
