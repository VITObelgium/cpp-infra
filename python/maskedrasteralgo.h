#pragma once

#include "gdx/algo/normalise.h"
#include "gdx/maskedraster.h"

namespace gdx {

template <typename TOutput, typename TInput>
MaskedRaster<TOutput> normalise(const MaskedRaster<TInput>& raster)
{
    auto meta = raster.metadata();
    if (meta.nodata.has_value()) {
        meta.nodata = std::numeric_limits<TOutput>::max();
    }
    MaskedRaster<TOutput> result(meta);
    normalise(raster, result);
    return result;
}
}
