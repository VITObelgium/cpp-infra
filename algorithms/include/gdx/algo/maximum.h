#pragma once

#include "gdx/exception.h"

#include <algorithm>

namespace gdx {

template <typename RasterType>
auto maximum(const RasterType& ras) -> typename RasterType::value_type
{
    auto iter = std::max_element(value_begin(ras), value_end(ras));
    if (iter == value_end(ras)) {
        throw InvalidArgument("No data in raster");
    }

    return *iter;
}

template <typename RasterType>
RasterType maximum(const std::vector<const RasterType*>& rasters)
{
    using T = typename RasterType::value_type;

    if (rasters.empty()) {
        throw InvalidArgument("No rasters provided");
    }

    if (rasters.size() == 1) {
        throw InvalidArgument("At least two rasters need to be provided");
    }

    const auto size = rasters.front()->size();
    auto resultMeta = rasters.front()->metadata();
    RasterType result(resultMeta);

    for (auto* ras : rasters) {
        if (ras->size() != size) {
            throw InvalidArgument("Not all provided rasters have the same size");
        }
    }

    std::vector<T> values(rasters.size());

    for (int32_t i = 0; i < size; ++i) {
        bool nodata = false;
        for (size_t r = 0; r < rasters.size(); ++r) {
            if (rasters[r]->is_nodata(i)) {
                assert(resultMeta.nodata.has_value());
                nodata = true;
            }

            values[r] = (*rasters[r])[i];
        }

        if (nodata) {
            result.mark_as_nodata(i);
        } else {
            result[i] = *std::max_element(values.begin(), values.end());
        }
    }

    return result;
}

#ifdef HAVE_OPENCL

template <typename T>
T maximum(const gpu::Raster<T>& ras)
{
    return *gpu::Policy<T>::max_element(ras.begin(), ras.end(), ras.metadata().nodata);
}
#endif
}
