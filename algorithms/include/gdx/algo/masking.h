#pragma once

#include "gdx/algo/algorithm.h"
#include "gdx/exception.h"

#include <algorithm>
#include <optional>

namespace gdx {

template <typename RasterType, typename MaskType>
void applyMask(RasterType& ras, const MaskType& mask)
{
    gdx::for_each_optional_value(ras, mask, [&](auto& rasterValue, auto& maskValue) {
        if (maskValue.is_nodata()) {
            rasterValue.mark_as_nodata();
        }
    });
}
}
