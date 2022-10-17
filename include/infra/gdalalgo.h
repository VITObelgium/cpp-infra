#pragma once

#include "infra/gdal.h"
#include "infra/gdalresample.h"
#include "infra/geometadata.h"

#include <vector>

namespace inf::gdal {

struct WarpOptions
{
    ResampleAlgorithm resampleAlgo = ResampleAlgorithm::NearestNeighbour;
    PolygonCRef clipPolygon;
    std::optional<double> clipBlendDistance;
    std::vector<std::string> additionalOptions;
};

void warp(const RasterDataSet& srcDataSet, RasterDataSet& dstDataSet, ResampleAlgorithm algo = ResampleAlgorithm::NearestNeighbour);
void warp(const RasterDataSet& srcDataSet, RasterDataSet& dstDataSet, WarpOptions& options);

// This version uses the same options as the command line tool
void warp(const RasterDataSet& srcDataSet, RasterDataSet& dstDataSet, const std::vector<std::pair<std::string, std::string>>& options);

VectorDataSet warp_vector(const fs::path& vectorPath, const GeoMetadata& destMeta, const std::vector<std::string>& options = {});
VectorDataSet warp_vector(const fs::path& vectorPath, const std::string& projection, const std::vector<std::string>& options = {});
VectorDataSet warp(const VectorDataSet& srcDataSet, const std::string& projection, const std::vector<std::string>& options = {});
VectorDataSet warp(const VectorDataSet& srcDataSet, const GeoMetadata& destMeta, const std::vector<std::string>& options = {});

/*! Returns the metadata of a raster when it would be warped, call this function before the
 *  call to warp_raster so you know the destination size and can allocate a buffer
 */
GeoMetadata warp_metadata(const GeoMetadata& meta, const std::string& destProjection);
GeoMetadata warp_metadata(const GeoMetadata& meta, int32_t destCrs);

// Create a vector dataset from a raster dataset
VectorDataSet polygonize(const RasterDataSet& ds);

// Create a raster dataset from a vector dataset
template <typename T>
std::pair<GeoMetadata, std::vector<T>> rasterize(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options = {});

// convert a vector dataset
VectorDataSet translate_vector(const VectorDataSet& ds, const std::vector<std::string>& options = {});
VectorDataSet translate_vector_to_disk(const VectorDataSet& ds, const fs::path& path, const std::vector<std::string>& options = {});

struct BufferOptions
{
    double distance         = 0.0;
    int32_t numQuadSegments = 30;
    bool includeFields;                         //! copy over the fields in the resulting dataset
    std::string attributeFilter;                //! apply an attribute filter to the input layers;
    std::optional<Geometry::Type> geometryType; //! override the type of the resulting geometry
};

VectorDataSet buffer_vector(VectorDataSet& ds, const BufferOptions opts);

// convert a raster dataset
template <typename T>
std::pair<GeoMetadata, std::vector<T>> translate(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options = {});

gdal::RasterDataSet translate(const fs::path& path, const std::vector<std::string>& options, const ProgressInfo::Callback& progressCb = nullptr);
gdal::RasterDataSet translate(const fs::path& path, const fs::path& outputPath, const std::vector<std::string>& options, const ProgressInfo::Callback& progressCb = nullptr);
gdal::RasterDataSet translate(const gdal::RasterDataSet& ds, const std::vector<std::string>& options, const ProgressInfo::Callback& progressCb = nullptr);
gdal::RasterDataSet translate(const gdal::RasterDataSet& ds, const fs::path& outputPath, const std::vector<std::string>& options, const ProgressInfo::Callback& progressCb = nullptr);

}
