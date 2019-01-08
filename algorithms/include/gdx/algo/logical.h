#pragma once

#include <algorithm>

namespace gdx {

template <typename RasterType>
bool all_of(const RasterType& ras)
{
    return std::all_of(value_begin(ras), value_end(ras), [](auto& v) {
        return v != 0;
    });
}

template <typename RasterType>
bool any_of(const RasterType& ras)
{
    return std::any_of(value_begin(ras), value_end(ras), [](auto& v) {
        return v != 0;
    });
}
}
