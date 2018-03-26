#pragma once

#include "infra/gdal.h"
#include "infra/geometadata.h"

namespace infra::gdal {

// Create a vector dataset from a raster dataset
DataSet polygonize(const DataSet& ds);

// Create a raster dataset from a vector dataset
template <typename T>
std::pair<GeoMetadata, std::vector<T>> rasterize(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options = {});

// convert a vector dataset
DataSet translateVector(const DataSet& ds, const std::vector<std::string>& options = {});

// convert a raster dataset
template <typename T>
std::pair<GeoMetadata, std::vector<T>> translate(const DataSet& ds, const GeoMetadata& meta, const std::vector<std::string>& options = {});
}
