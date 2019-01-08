#pragma once

#include "gdx/cell.h"
#include "gdx/exception.h"

#include <algorithm>
#include <vector>

namespace gdx {

template <typename RasterType>
auto minimum(const RasterType& ras) -> typename RasterType::value_type
{
    auto iter = std::min_element(value_begin(ras), value_end(ras));
    if (iter == value_end(ras)) {
        throw InvalidArgument("No data in raster");
    }

    return *iter;
}

template <typename RasterType>
auto minmax(const RasterType& ras) -> std::pair<typename RasterType::value_type, typename RasterType::value_type>
{
    auto [minIter, maxIter] = std::minmax_element(value_begin(ras), value_end(ras));
    if (minIter == value_end(ras) || maxIter == value_end(ras)) {
        throw InvalidArgument("No data in raster");
    }

    return {*minIter, *maxIter};
}

template <typename RasterType>
std::pair<Cell, Cell> minmax_cell(const RasterType& ras)
{
    auto [minIter, maxIter] = std::minmax_element(value_begin(ras), value_end(ras));
    if (minIter == value_end(ras) || maxIter == value_end(ras)) {
        throw InvalidArgument("No data in raster");
    }

    return {minIter.cell(), maxIter.cell()};
}

template <typename RasterType>
RasterType minimum(const std::vector<const RasterType*>& rasters)
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
            result[i] = *std::min_element(values.begin(), values.end());
        }
    }

    return result;
}
}
