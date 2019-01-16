#pragma once

#include "gdx/denseraster.h"
#include "pointsourcemodel.h"

namespace opaq {
using RasterPtr      = std::shared_ptr<gdx::DenseRaster<double>>;
using PointSourcePtr = std::shared_ptr<std::vector<PointSourceModelData>>;
}

// Make sure we can store our enums and types in QVariants in a type safe way
//Q_DECLARE_METATYPE(opaq::RasterPtr)
