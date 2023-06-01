#pragma once

#include "infra/cast.h"
#include "infra/colormap.h"
#include "infra/enumutils.h"
#include "infra/exception.h"
#include "infra/filesystem.h"
#include "infra/gdal.h"
#include "infra/geometadata.h"
#include "infra/point.h"
#include "infra/span.h"

#include <cassert>
#include <fmt/format.h>
#include <gdal_version.h>
#include <limits>
#include <ogr_spatialref.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace inf::gdal::io {

GeoMetadata read_metadata(const fs::path& fileName, const std::vector<std::string>& driverOpts = {});
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
void create_output_directory_if_needed(const fs::path& p);

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

    if constexpr (std::is_same_v<StorageType, RasterDataType>) {
        write_raster_dataset(data, memDataSet, meta, filename, driverOptions, metadataValues, ct);
    } else {
        // TODO: Investigate VRT driver to create a virtual dataset with different type without creating a copy
        std::vector<StorageType> converted(data.size());
        std::transform(data.begin(), data.end(), converted.begin(), [](RasterDataType d) { return static_cast<StorageType>(d); });
        write_raster_dataset<StorageType>(converted, memDataSet, meta, filename, driverOptions, metadataValues, ct);
    }
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
    switch (resolve_type(storageType)) {
    case GDT_Byte:
        write_raster_data<uint8_t, RasterDataType>(data, meta, filename, driverOptions, metadataValues, ct);
        break;
    case GDT_UInt16:
        write_raster_data<uint16_t, RasterDataType>(data, meta, filename, driverOptions, metadataValues, ct);
        break;
    case GDT_Int16:
        write_raster_data<int16_t, RasterDataType>(data, meta, filename, driverOptions, metadataValues, ct);
        break;
    case GDT_UInt32:
        write_raster_data<uint32_t, RasterDataType>(data, meta, filename, driverOptions, metadataValues, ct);
        break;
    case GDT_Int32:
        write_raster_data<int32_t, RasterDataType>(data, meta, filename, driverOptions, metadataValues, ct);
        break;
#if GDAL_VERSION_MAJOR > 2
    case GDT_UInt64:
        write_raster_data<uint64_t, RasterDataType>(data, meta, filename, driverOptions, metadataValues, ct);
        break;
    case GDT_Int64:
        write_raster_data<int64_t, RasterDataType>(data, meta, filename, driverOptions, metadataValues, ct);
        break;
#endif
    case GDT_Float32:
        write_raster_data<float, RasterDataType>(data, meta, filename, driverOptions, metadataValues, ct);
        break;
    case GDT_Float64:
        write_raster_data<double, RasterDataType>(data, meta, filename, driverOptions, metadataValues, ct);
        break;
    case GDT_CInt16:
    case GDT_CInt32:
    case GDT_CFloat32:
    case GDT_CFloat64:
    case GDT_Unknown:
    default:
        throw InvalidArgument("Unsupported storage type");
    }
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

inline CutOut intersect_metadata(const GeoMetadata& srcMeta, const GeoMetadata& dstMeta)
{
    // srcMeta: the metadata of the raster that we are going to read as it is on disk
    // dstMeta: the metadata of the raster that will be returned to the user

    if (!math::approx_equal(srcMeta.cellSize.x, dstMeta.cellSize.x, 1e-10) || !math::approx_equal(srcMeta.cellSize.y, dstMeta.cellSize.y, 1e-10)) {
        throw InvalidArgument("Extents cellsize does not match {} <-> {}", srcMeta.cellSize, dstMeta.cellSize);
    }

    if (srcMeta.cellSize.x == 0) {
        throw InvalidArgument("Extents cellsize is zero");
    }

    auto cellSize  = srcMeta.cellSize;
    auto srcBBox   = srcMeta.bounding_box();
    auto dstBBox   = dstMeta.bounding_box();
    auto intersect = rectangle_intersection(srcBBox, dstBBox);

    CutOut result;

    result.srcColOffset = srcMeta.convert_x_to_col(dstMeta.convert_col_centre_to_x(0));
    result.srcRowOffset = srcMeta.convert_y_to_row(dstMeta.convert_row_centre_to_y(0));
    result.rows         = static_cast<int>(std::abs(std::round(intersect.height() / cellSize.y)));
    result.cols         = static_cast<int>(std::round(intersect.width() / cellSize.x));
    result.dstColOffset = static_cast<int>(std::round((intersect.topLeft.x - dstBBox.topLeft.x) / cellSize.x));
    result.dstRowOffset = static_cast<int>(std::round((dstBBox.topLeft.y - intersect.topLeft.y) / std::abs(cellSize.y)));

    return result;
}
}

template <typename T>
gdal::RasterDataSet create_memory_dataset(std::span<const T> rasterData, const GeoMetadata& meta)
{
    auto memDriver = gdal::RasterDriver::create(gdal::RasterType::Memory);
    gdal::RasterDataSet srcDataSet(memDriver.create_dataset<T>(meta.rows, meta.cols, 0));
    srcDataSet.add_band(rasterData.data());
    srcDataSet.write_geometadata(meta);
    return srcDataSet;
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

        if (typeid(T()) != dataSet.band_datatype(bandNr)) {
            // perform nodata sanity checks
            if (auto nodata = dataSet.nodata_value(bandNr); nodata.has_value()) {
                if (std::isfinite(*nodata) && !fits_in_type<T>(*nodata)) {
                    // gdal performs a cast, so assign the nodata to the cast result
                    dstMeta.nodata = double(static_cast<T>(*nodata));
                }
            }
        }
    }

    return dstMeta;
}

/* This version will read the full dataset and is used in cases where there is no geotransform info available */
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
    create_output_directory_if_needed(filename);

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
    create_output_directory_if_needed(filename);

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

}
