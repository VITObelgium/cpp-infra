#pragma once

#include "gdx/algo/algorithm.h"

#include <cmath>

namespace gdx {

template <typename InputRaster, typename OutputRaster>
void is_nodata(const InputRaster& input, OutputRaster& result)
{
    using TDest = typename OutputRaster::value_type;
    std::transform(optional_value_begin(input), optional_value_end(input), optional_value_begin(result), [](auto& value) {
        return TDest(value ? 0 : 1);
    });
}

template <typename InputRaster, typename OutputRaster>
void is_data(const InputRaster& input, OutputRaster& result)
{
    using TDest = typename OutputRaster::value_type;
    std::transform(optional_value_begin(input), optional_value_end(input), optional_value_begin(result), [](auto& value) {
        return TDest(value ? 1 : 0);
    });
}

// Fills the provided raster completely with nodata
template <typename RasterType>
void make_nodata(RasterType& ras)
{
    using T = typename RasterType::value_type;
    std::fill(optional_value_begin(ras), optional_value_end(ras), std::optional<T>());
}

template <typename RasterType>
void replace_nodata_in_place(RasterType& ras, typename RasterType::value_type newValue)
{
    std::for_each(optional_value_begin(ras), optional_value_end(ras), [newValue](auto& value) {
        if (!value) {
            value = newValue;
        }
    });
}

template <typename RasterType>
void turn_value_into_nodata(RasterType& ras, typename RasterType::value_type valueToReplace)
{
    if (!ras.has_nodata()) {
        throw RuntimeError("No nodata value defined for raster");
    }

    std::for_each(optional_value_begin(ras), optional_value_end(ras), [valueToReplace](auto& value) {
        if (value && *value == valueToReplace) {
            value.reset();
        }
    });
}

template <template <typename> typename RasterType, typename T>
RasterType<T> replace_nodata(const RasterType<T>& ras, const T newValue)
{
    auto result = ras.copy();
    replace_nodata_in_place(result, newValue);
    return result;
}
}
