#pragma once

#include "infra/gdal.h"
#include "infra/geometadata.h"

#include <gdalwarper.h>

namespace inf::gdal {

/* Note: values are selected to be consistent with GDALRIOResampleAlg of gcore/gdal.h */
/*! Warp Resampling Algorithm */
enum class ResampleAlgorithm
{
    NearestNeighbour = GRA_NearestNeighbour, // Nearest neighbour (select on one input pixel)
    Bilinear         = GRA_Bilinear,         // Bilinear (2x2 kernel)
    Cubic            = GRA_Cubic,            // Cubic Convolution Approximation (4x4 kernel)
    CubicSpline      = GRA_CubicSpline,      // Cubic B-Spline Approximation (4x4 kernel)
    Lanczos          = GRA_Lanczos,          // Lanczos windowed sinc interpolation (6x6 kernel)
    Average          = GRA_Average,          // Average (computes the average of all non-NODATA contributing pixels)
    Mode             = GRA_Mode,             // Mode (selects the value which appears most often of all the sampled points)
    Maximum          = GRA_Max,              // Max (selects maximum of all non-NODATA contributing pixels)
    Minimum          = GRA_Min,              // Min (selects minimum of all non-NODATA contributing pixels)
    Median           = GRA_Med,              // Med (selects median of all non-NODATA contributing pixels)
    FirstQuantile    = GRA_Q1,               // Q1 (selects first quartile of all non-NODATA contributing pixels)
    ThirdQuantile    = GRA_Q3,               // Q3 (selects third quartile of all non-NODATA contributing pixels)
#if GDAL_VERSION_NUM >= 3010000
    Sum            = GRA_Sum, // Weighted sum (weighed sum of all non-NODATA contributing pixels)
    RootMeanSquare = GRA_RMS, // Root mean square (weighted root mean square (quadratic mean) of all non-NODATA contributing pixels)
#endif
};

struct WarpOptions
{
    ResampleAlgorithm resampleAlgo = ResampleAlgorithm::NearestNeighbour;
    PolygonCRef clipPolygon;
    std::optional<double> clipBlendDistance;
};

void warp(const RasterDataSet& srcDataSet, RasterDataSet& dstDataSet, ResampleAlgorithm algo = ResampleAlgorithm::NearestNeighbour);
void warp(const RasterDataSet& srcDataSet, RasterDataSet& dstDataSet, WarpOptions& options);

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

}
