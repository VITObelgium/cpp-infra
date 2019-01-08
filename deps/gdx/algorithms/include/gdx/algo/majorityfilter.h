#pragma once

#include "gdx/algo/suminbuffer.h"

#include <functional>
#include <map>
#include <set>
#include <tuple>

namespace gdx {

template <template <typename> typename RasterType, typename T>
std::optional<T> computeMajority(const Cell cell, const RasterType<T>& oldValue, int32_t radius, int32_t radius2, const RasterMetadata& meta,
    std::map<T, int32_t>& count)
{
    std::optional<T> result;
    count.clear();

    const int r2 = radius2;
    int dr, dc, d2;
    int nodataCount = 0;
    for (int rr = cell.r - radius; rr <= cell.r + radius; ++rr) {
        for (int cc = cell.c - radius; cc <= cell.c + radius; ++cc) {
            if (!meta.is_on_map(rr, cc)) {
                continue;
            }

            dr = rr - cell.r;
            dc = cc - cell.c;
            d2 = dr * dr + dc * dc;
            if (d2 <= r2) {
                if (oldValue.is_nodata(rr, cc)) {
                    ++nodataCount;
                } else {
                    ++(count[oldValue(rr, cc)]);
                }
            }
        }
    }

    int maxCount = oldValue.is_nodata(cell) ? 0 : count[oldValue[cell]]; // in case of equal counts take the present value if applicable
    for (auto iter = count.begin(); iter != count.end(); ++iter) {
        if (maxCount < iter->second) {
            assert(iter->second != 0);
            maxCount = iter->second;
            result   = iter->first;
        }
    }

    if (!result && maxCount != 0) {
        result = oldValue[cell];
    }

    return result;
}

template <template <typename> typename RasterType, typename T>
RasterType<T> majorityFilter(const RasterType<T>& ras, float radiusInMeter)
{
    if (ras.metadata().cellSize == 0) {
        throw RuntimeError("Radius is undefined when the cell size is 0");
    }

    const auto rows           = ras.rows();
    const auto cols           = ras.cols();
    auto resultMeta           = ras.metadata();
    float radiusInCellsFloat  = static_cast<float>(radiusInMeter / ras.metadata().cellSize);
    const auto radiusInCells  = static_cast<int32_t>(radiusInCellsFloat);
    const auto radiusInCells2 = static_cast<int32_t>(radiusInCellsFloat * radiusInCellsFloat);
    RasterType<T> result(resultMeta);

    std::map<T, int32_t> count;
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            auto majority = computeMajority(Cell(r, c), ras, radiusInCells, radiusInCells2, resultMeta, count);
            if (majority.has_value()) {
                result(r, c) = majority.value();
            } else {
                result.mark_as_nodata(r, c);
            }
        }
    }

    return result;
}
}
