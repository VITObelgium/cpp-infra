#pragma once

#include "infra/gdal.h"

namespace infra::gdal {

// Create a vector dataset from a raster dataset
DataSet polygonize(const DataSet& ds);
}
