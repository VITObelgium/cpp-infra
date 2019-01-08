#pragma once

#include "gdx/exception.h"

namespace gdx {
template <typename RasterType1, typename RasterType2>
void throw_on_size_mismatch(const RasterType1& lhs, const RasterType2& rhs)
{
    if (lhs.rows() != rhs.rows() || lhs.cols() != rhs.cols()) {
        throw InvalidArgument("Raster dimensions must match: {}x{} vs {}x{}", lhs.rows(), lhs.cols(), rhs.rows(), rhs.cols());
    }
}
}
