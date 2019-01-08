#pragma once

#include "gdx/algo/algorithm.h"
#include "infra/cast.h"

namespace gdx {

template <typename TOutput, template <typename> typename RasterType, typename TInput>
auto raster_cast(const RasterType<TInput>& input) -> RasterType<TOutput>
{
    constexpr bool outputHasNaN = RasterType<TOutput>::raster_type_has_nan;

    auto resultMeta = input.metadata();
    if constexpr (!outputHasNaN) {
        if (resultMeta.nodata) {
            if (std::isnan(*resultMeta.nodata)) {
                // modify the NaN nodata when the resulting type does not support NaN values
                resultMeta.nodata = static_cast<TOutput>(*resultMeta.nodata);
            } else if (!inf::fits_in_type<TOutput>(resultMeta.nodata.value())) {
                resultMeta.nodata = std::numeric_limits<TOutput>::max();
            }
        }
    }

    RasterType<TOutput> output(std::move(resultMeta));
    std::transform(optional_value_begin(input), optional_value_end(input), optional_value_begin(output), [](auto& value) -> std::optional<TOutput> {
        if (value) {
            return static_cast<TOutput>(*value);
        } else {
            return std::optional<TOutput>();
        }
    });

    return output;
}
}
