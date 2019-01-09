#pragma once

#include "gdx/denseraster.h"

namespace opaq {
using RasterPtr = std::shared_ptr<gdx::DenseRaster<double>>;
}

// Make sure we can store our enums and types in QVariants in a type safe way
Q_DECLARE_METATYPE(opaq::RasterPtr)
