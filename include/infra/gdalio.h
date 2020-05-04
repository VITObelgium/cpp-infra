#pragma once

#include "infra/cast.h"
#include "infra/colormap.h"
#include "infra/enumutils.h"
#include "infra/exception.h"
#include "infra/filesystem.h"
#include "infra/gdal.h"
#include "infra/gdalalgo.h"
#include "infra/geometadata.h"
#include "infra/point.h"
#include "infra/span.h"

#include <fmt/format.h>
#include <limits>
#include <ogr_spatialref.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace inf::gdal::io {

GeoMetadata read_metadata(const fs::path& fileName);
const std::type_info& get_raster_type(const fs::path& fileName);

namespace detail {

using BBox = inf::Rect<double>;

struct CutOut
{
    int srcColOffset = 0;
    int srcRowOffset = 0;
    int dstColOffset = 0;
    int dstRowOffset = 0;
    int rows         = 0;
    int cols         = 0;
};

template <typename T>
void read_raster_data(int bandNr, CutOut cut, const gdal::RasterDataSet& dataSet, T* data, int dataCols)
{
    if (cut.dstRowOffset > 0) {
        data += cut.dstRowOffset * dataCols;
    }

    if (cut.dstColOffset > 0) {
        data += cut.dstColOffset;
    }

    dataSet.read_rasterdata<T>(bandNr, std::max(0, cut.srcColOffset), std::max(0, cut.srcRowOffset), cut.cols, cut.rows, data, cut.cols, cut.rows, 0, dataCols * sizeof(T));
}

void read_raster_data(int bandNr, CutOut cut, const gdal::RasterDataSet& dataSet, const std::type_info& info, void* data, int dataCols);

template <typename RasterDataType>
void write_raster_dataset(
    std::span<const RasterDataType> data,
    gdal::RasterDataSet& memDataSet,
    const GeoMetadata& meta,
    const fs::path& filename,
    std::span<const std::string> driverOptions,
    const std::unordered_map<std::string, std::string>& metadataValues,
    const GDALColorTable* ct = nullptr)
{
    memDataSet.add_band(data.data());
    memDataSet.set_colortable(1, ct);
    memDataSet.write_geometadata(meta);

    auto driver = gdal::RasterDriver::create(filename);
    std::vector<std::string> options;
    if (driverOptions.empty() && driver.type() == gdal::RasterType::GeoTiff) {
        options.emplace_back("COMPRESS=LZW");
        options.emplace_back("TILED=YES");
        options.emplace_back("NUM_THREADS=ALL_CPUS");
        driverOptions = options;
    }

    for (auto& [key, value] : metadataValues) {
        memDataSet.set_metadata(key, value);
    }

    driver.create_dataset_copy(memDataSet, filename, driverOptions);
}

template <typename RasterDataType>
void write_raster_to_dataset_band(
    inf::gdal::RasterDataSet& ds,
    int bandNr,
    std::span<const RasterDataType> data,
    const GeoMetadata& meta)
{
    auto nod = ds.nodata_value(bandNr);
    if (nod != meta.nodata) {
        // don't update when both values have nan value
        if (!(meta.nodata.has_value() && nod.has_value() && std::isnan(meta.nodata.value()) && std::isnan(nod.value()))) {
            ds.set_nodata_value(bandNr, meta.nodata);
        }
    }
    ds.write_rasterdata(bandNr, 0, 0, meta.cols, meta.rows, data.data(), meta.cols, meta.rows);
}

template <typename RasterDataType>
void write_raster_data(
    std::span<const RasterDataType> data,
    const GeoMetadata& meta,
    const fs::path& filename,
    const std::type_info& storageType,
    std::span<const std::string> driverOptions,
    const std::unordered_map<std::string, std::string>& metadataValues,
    const GDALColorTable* ct = nullptr)
{
    // To write a raster to disk we need a dataset that contains the data
    // Create a memory dataset with 0 bands, then assign a band given the pointer of our vector
    // Creating a dataset with 1 band would casuse unnecessary memory allocation
    auto memDriver = gdal::RasterDriver::create(gdal::RasterType::Memory);
    gdal::RasterDataSet memDataSet(memDriver.create_dataset(meta.rows, meta.cols, 0, storageType));
    write_raster_dataset(data, memDataSet, meta, filename, driverOptions, metadataValues, ct);
}

template <typename StorageType, typename RasterDataType>
void write_raster_data(
    std::span<const RasterDataType> data,
    const GeoMetadata& meta,
    const fs::path& filename,
    std::span<const std::string> driverOptions,
    const std::unordered_map<std::string, std::string>& metadataValues,
    const GDALColorTable* ct = nullptr)
{
    // To write a raster to disk we need a dataset that contains the data
    // Create a memory dataset with 0 bands, then assign a band given the pointer of our vector
    // Creating a dataset with 1 band would casuse unnecessary memory allocation
    auto memDriver = gdal::RasterDriver::create(gdal::RasterType::Memory);
    gdal::RasterDataSet memDataSet(memDriver.create_dataset<StorageType>(meta.rows, meta.cols, 0));
    write_raster_dataset(data, memDataSet, meta, filename, driverOptions, metadataValues, ct);
}

template <typename StorageType, typename RasterType>
void write_raster_data(
    const RasterType& raster,
    const GeoMetadata& meta,
    const fs::path& filename,
    std::span<const std::string> driverOptions,
    const std::unordered_map<std::string, std::string>& metadataValues,
    const GDALColorTable* ct = nullptr)
{
    write_raster_data<StorageType>(std::span<const typename RasterType::value_type>(data(raster), size(raster)), meta, filename, driverOptions, metadataValues, ct);
}

static BBox metadata_bounding_box(const GeoMetadata& meta)
{
    BBox result;
    auto width         = meta.cellSize * meta.cols;
    auto height        = meta.cellSize * meta.rows;
    result.topLeft     = Point<double>(meta.xll, meta.yll + height);
    result.bottomRight = Point<double>(result.topLeft.x + width, result.topLeft.y - height);
    return result;
}

inline BBox intersect_bbox(const BBox& lhs, const BBox& rhs)
{
    auto topLeft     = Point<double>(std::max(lhs.topLeft.x, rhs.topLeft.x), std::min(lhs.topLeft.y, rhs.topLeft.y));
    auto bottomRight = Point<double>(std::min(lhs.bottomRight.x, rhs.bottomRight.x), std::max(lhs.bottomRight.y, rhs.bottomRight.y));
    return {topLeft, bottomRight};
}

inline CutOut intersect_metadata(const GeoMetadata& srcMeta, const GeoMetadata& dstMeta)
{
    // srcMeta: the metadata of the raster that we are going to read as it is on disk
    // dstMeta: the metadata of the raster that will be returned to the user

    if (srcMeta.cellSize != dstMeta.cellSize) {
        throw InvalidArgument("Extents cellsize does not match {} <-> {}", srcMeta.cellSize, dstMeta.cellSize);
    }

    if (srcMeta.cellSize == 0) {
        throw InvalidArgument("Extents cellsize is zero");
    }

    auto cellSize  = srcMeta.cellSize;
    auto srcBBox   = metadata_bounding_box(srcMeta);
    auto dstBBox   = metadata_bounding_box(dstMeta);
    auto intersect = intersect_bbox(srcBBox, dstBBox);

    CutOut result;

    result.srcColOffset = srcMeta.convert_x_to_col(dstMeta.convert_col_centre_to_x(0));
    result.srcRowOffset = srcMeta.convert_y_to_row(dstMeta.convert_row_centre_to_y(0));
    result.rows         = static_cast<int>(std::round(intersect.height() / cellSize));
    result.cols         = static_cast<int>(std::round(intersect.width() / cellSize));
    result.dstColOffset = static_cast<int>(std::round((intersect.topLeft.x - dstBBox.topLeft.x) / cellSize));
    result.dstRowOffset = static_cast<int>(std::round((dstBBox.topLeft.y - intersect.topLeft.y) / cellSize));

    return result;
}
}

template <typename TSource, typename TDest>
GeoMetadata cast_raster(const GeoMetadata& meta, std::span<const TSource> srcData, std::span<TDest> dstData)
{
    using namespace detail;
    constexpr bool srcHasNaN = std::numeric_limits<TSource>::has_quiet_NaN;
    constexpr bool dstHasNaN = std::numeric_limits<TDest>::has_quiet_NaN;

    auto resultMeta = meta;
    if constexpr (!dstHasNaN) {
        if (resultMeta.nodata) {
            if (std::isnan(*resultMeta.nodata)) {
                // modify the NaN nodata when the resulting type does not support NaN values
                resultMeta.nodata = static_cast<TDest>(*resultMeta.nodata);
            } else if (!inf::fits_in_type<TDest>(resultMeta.nodata.value())) {
                resultMeta.nodata = std::numeric_limits<TDest>::max();
            }
        }
    }

    if constexpr (!srcHasNaN && dstHasNaN) {
        if (meta.nodata) {
            auto nodata = *meta.nodata;
            // nodata values will be replaced with nan
            for (int32_t i = 0; i < srcData.size(); ++i) {
                if (srcData[i] == nodata) {
                    dstData[i] = std::numeric_limits<TDest>::quiet_NaN();
                } else {
                    dstData[i] = static_cast<TDest>(srcData[i]);
                }
            }

            return resultMeta;
        }
    } else if constexpr (srcHasNaN && !dstHasNaN) {
        if (resultMeta.nodata) {
            auto nodata = static_cast<TDest>(*resultMeta.nodata);
            // nan values need to be replaced with the nodata value
            for (size_t i = 0; i < srcData.size(); ++i) {
                if (std::isnan(srcData[i]) || srcData[i] == meta.nodata) {
                    dstData[i] = nodata;
                } else {
                    dstData[i] = static_cast<TDest>(srcData[i]);
                }
            }

            return resultMeta;
        }
    }

    // No nodata conversions, regular copy
    for (size_t i = 0; i < srcData.size(); ++i) {
        dstData[i] = static_cast<TDest>(srcData[i]);
    }

    return resultMeta;
}

/*! The provided extent will be the extent of the resulting raster
 * Areas outside the extent of the raster on disk will be filled with nodata
 */
template <typename T>
GeoMetadata data_from_dataset(const gdal::RasterDataSet& dataSet, const GeoMetadata& extent, int bandNr, std::span<T> dstData)
{
    using namespace detail;

    auto meta   = dataSet.geometadata(bandNr);
    auto cutOut = intersect_metadata(meta, extent);

    bool cutOutSmallerThenExtent = (extent.rows * extent.cols) != (cutOut.rows * cutOut.cols);
    auto dstMeta                 = extent;
    if (meta.nodata.has_value()) {
        dstMeta.nodata = meta.nodata;
    }

    if (cutOutSmallerThenExtent && !dstMeta.nodata.has_value()) {
        dstMeta.nodata = static_cast<double>(std::numeric_limits<T>::max());
    }

    if (truncate<int32_t>(dstData.size()) != dstMeta.rows * dstMeta.cols) {
        throw InvalidArgument("Invalid data buffer provided: incorrect size");
    }

    if (cutOutSmallerThenExtent && dstMeta.nodata.has_value()) {
        std::fill(dstData.begin(), dstData.end(), static_cast<T>(dstMeta.nodata.value()));
    }

    bool isByte = std::is_same_v<T, uint8_t>;
    if (isByte && dstMeta.nodata.has_value() && !inf::fits_in_type<T>(dstMeta.nodata.value())) {
        std::vector<float> tempData(extent.rows * extent.cols, static_cast<float>(dstMeta.nodata.value_or(0)));
        read_raster_data(bandNr, cutOut, dataSet, tempData.data(), extent.cols);
        dstMeta = cast_raster<float, T>(dstMeta, tempData, dstData);
    } else {
        read_raster_data(bandNr, cutOut, dataSet, dstData.data(), dstMeta.cols);
    }

    return dstMeta;
}

/* This version will read the full dataset and is used in cases where there is no geotransform info available
 */
template <typename T>
GeoMetadata data_from_dataset(const gdal::RasterDataSet& dataSet, int bandNr, std::span<T> dstData)
{
    using namespace detail;

    GeoMetadata meta;
    meta.nodata = dataSet.nodata_value(bandNr);
    meta.cols   = dataSet.x_size();
    meta.rows   = dataSet.y_size();

    if (truncate<int32_t>(dstData.size()) != meta.rows * meta.cols) {
        throw InvalidArgument("Invalid data buffer provided: incorrect size");
    }

    CutOut cutOut;
    cutOut.rows = meta.rows;
    cutOut.cols = meta.cols;

    bool isByte = std::is_same_v<T, uint8_t>;
    if (isByte && meta.nodata.has_value() && !inf::fits_in_type<T>(meta.nodata.value())) {
        std::vector<float> tempData(meta.rows * meta.cols, static_cast<float>(meta.nodata.value_or(0)));
        read_raster_data(bandNr, cutOut, dataSet, tempData.data(), meta.cols);
        meta = cast_raster<float, T>(meta, tempData, dstData);
    } else {
        read_raster_data(bandNr, cutOut, dataSet, dstData.data(), meta.cols);
    }

    return meta;
}

template <typename T>
GeoMetadata read_raster_data(gdal::RasterDataSet& dataSet, int bandNr, std::span<T> dstData)
{
    if (!dataSet.has_valid_geotransform()) {
        return data_from_dataset(dataSet, bandNr, dstData);
    } else {
        return data_from_dataset(dataSet, dataSet.geometadata(bandNr), bandNr, dstData);
    }
}

template <typename T>
GeoMetadata read_raster_data(gdal::RasterDataSet& dataSet, std::span<T> dstData)
{
    return read_raster_data<T>(dataSet, 1, dstData);
}

template <typename T>
GeoMetadata read_raster_data(gdal::RasterDataSet& dataSet, const GeoMetadata& extent, int bandNr, std::span<T> dstData)
{
    return data_from_dataset(dataSet, extent, bandNr, dstData);
}

template <typename T>
GeoMetadata read_raster_data(gdal::RasterDataSet& dataSet, const GeoMetadata& extent, std::span<T> dstData)
{
    return read_raster_data<T>(dataSet, extent, 1, dstData);
}

template <typename StorageType, class RasterType>
void write_raster_as(std::span<const RasterType> rasterData, const GeoMetadata& meta, const fs::path& filename, std::span<const std::string> driverOptions = {}, const std::unordered_map<std::string, std::string>& metadataValues = {})
{
    using namespace detail;
    inf::file::create_directory_if_not_exists(filename.parent_path());

    if constexpr (std::is_unsigned_v<StorageType>) {
        auto nodata         = meta.nodata;
        auto negativeNodata = nodata.has_value() && nodata.value() < 0;
        if (negativeNodata) {
            throw RuntimeError("Trying to store a raster with unsigned data type using negative nodata value");
        }
    }

    write_raster_data<StorageType>(rasterData, meta, filename, driverOptions, metadataValues);
}

template <typename StorageType, class RasterType>
void write_raster_as(std::span<const RasterType> rasterData, const GeoMetadata& meta, inf::gdal::RasterDataSet& ds, int bandNr)
{
    detail::write_raster_to_dataset_band<StorageType>(ds, bandNr, rasterData, meta);
}

template <class T>
void write_raster(std::span<const T> rasterData, const GeoMetadata& meta, inf::gdal::RasterDataSet& ds, int bandNr)
{
    detail::write_raster_to_dataset_band<T>(ds, bandNr, rasterData, meta);
}

template <class T>
void write_raster(std::span<const T> rasterData, const GeoMetadata& meta, const fs::path& filename, const std::type_info& storageType, std::span<const std::string> driverOptions = {}, const std::unordered_map<std::string, std::string>& metadataValues = {})
{
    using namespace detail;
    inf::file::create_directory_if_not_exists(filename.parent_path());

    if (storageType == typeid(uint8_t) ||
        storageType == typeid(uint16_t) ||
        storageType == typeid(uint32_t) ||
        storageType == typeid(uint64_t)) {
        auto nodata         = meta.nodata;
        auto negativeNodata = nodata.has_value() && nodata.value() < 0;
        if (negativeNodata) {
            throw RuntimeError("Trying to store a raster with unsigned data type using negative nodata value");
        }
    }

    write_raster_data(rasterData, meta, filename, storageType, driverOptions, metadataValues);
}

template <class T>
void write_raster(std::span<const T> data, const GeoMetadata& meta, const fs::path& filename, std::span<const std::string> driverOptions = {})
{
    write_raster(data, meta, filename, typeid(T), driverOptions);
}

template <class T>
void write_raster_with_metadata(std::span<const T> data, const GeoMetadata& meta, const fs::path& filename, const std::unordered_map<std::string, std::string>& metadataValues, std::span<const std::string> driverOptions = {})
{
    write_raster_as<T>(data, meta, filename, driverOptions, metadataValues);
}

template <class T>
void write_raster_color_mapped(std::span<const T> data, const GeoMetadata& meta, const fs::path& filename, const inf::ColorMap& cm, std::span<const std::string> driverOptions = {})
{
    GDALColorTable ct;
    for (int i = 0; i < 256; ++i) {
        auto color = cm.get_color(static_cast<uint8_t>(i));

        GDALColorEntry entry;
        entry.c1 = color.r;
        entry.c2 = color.g;
        entry.c3 = color.b;
        entry.c4 = 255;

        ct.SetColorEntry(i, &entry);
    }

    detail::write_raster_data<T>(data, meta, filename, driverOptions, {}, &ct);
}

template <typename T>
inf::gdal::VectorDataSet polygonize(std::span<const T> rasterData, const GeoMetadata& meta)
{
    assert(meta.rows * meta.cols == rasterData.size());

    auto memDriver = gdal::RasterDriver::create(gdal::RasterType::Memory);
    gdal::RasterDataSet srcDataSet(memDriver.create_dataset<T>(meta.rows, meta.cols, 0));

    srcDataSet.add_band(rasterData.data());
    srcDataSet.write_geometadata(meta);

    return inf::gdal::polygonize(srcDataSet);
}

template <typename TInput, typename TOutput>
void warp_raster(std::span<const TInput> inputData, const GeoMetadata& inputMeta, std::span<TOutput> outputData, GeoMetadata outputMeta, gdal::ResampleAlgorithm algo = gdal::ResampleAlgorithm::NearestNeighbour)
{
    if (inputMeta.projection.empty()) {
        throw RuntimeError("Warp input raster does not contain projection information");
    }

    if (outputMeta.projection.empty()) {
        throw RuntimeError("Warp output raster does not contain projection information");
    }

    assert(outputData.size() == outputMeta.rows * outputMeta.cols);
    if (!outputMeta.nodata.has_value()) {
        if constexpr (std::numeric_limits<TOutput>::has_quiet_NaN) {
            outputMeta.nodata = std::numeric_limits<TOutput>::quiet_NaN();
        } else {
            outputMeta.nodata = std::numeric_limits<TOutput>::max();
        }
    }

    auto memDriver = gdal::RasterDriver::create(gdal::RasterType::Memory);
    gdal::RasterDataSet srcDataSet(memDriver.create_dataset<TInput>(inputMeta.rows, inputMeta.cols, 0));
    srcDataSet.add_band(inputData.data());
    srcDataSet.write_geometadata(inputMeta);

    gdal::RasterDataSet dstDataSet(memDriver.create_dataset<TOutput>(outputMeta.rows, outputMeta.cols, 0u, ""));
    dstDataSet.add_band(outputData.data());
    // Make sure the resulting data set has the proper projection information
    dstDataSet.write_geometadata(outputMeta);

    gdal::warp(srcDataSet, dstDataSet, algo);
}

}
