#pragma once

#include "infra/gdal.h"
#include "infra/geometadata.h"

namespace inf::gdal {

// Create a vector dataset from a raster dataset
VectorDataSet polygonize(const RasterDataSet& ds);

// Create a raster dataset from a vector dataset
template <typename T>
std::pair<GeoMetadata, std::vector<T>> rasterize(const VectorDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options = {});

// convert a vector dataset
VectorDataSet translate_vector(const VectorDataSet& ds, const std::vector<std::string>& options = {});

// convert a raster dataset
template <typename T>
std::pair<GeoMetadata, std::vector<T>> translate(const RasterDataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options = {});
}