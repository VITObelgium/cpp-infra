#pragma once

#include "gdx/algo/algorithm.h"
#include "gdx/algo/sum.h"

#include <functional>
#include <unordered_map>

namespace gdx {

template <
    typename RasterInputType,
    typename RasterOutputType,
    typename AreaRasterType>
void choroplethSum(const RasterInputType& input, const AreaRasterType& areas, RasterOutputType& output)
{
    using TAreas  = typename AreaRasterType::value_type;
    using TOutput = typename RasterOutputType::value_type;

    if (size(input) != size(output) || size(input) != size(areas)) {
        throw InvalidArgument("choropleth sum: raster sizes must match {} vs {} vs {}", size(input), size(areas), size(output));
    }

    auto sums = sumMask<TOutput>(input, areas);
    gdx::transform(areas, output, [&sums](TAreas value) {
        return static_cast<TOutput>(sums[value]);
    });
}

template <
    typename RasterInputType,
    typename RasterOutputType,
    typename AreaRasterType>
void choroplethAverage(const RasterInputType& input, const AreaRasterType& areas, RasterOutputType& output)
{
    using TRasterInput  = typename RasterInputType::value_type;
    using TRasterOutput = typename RasterOutputType::value_type;
    using TAreas        = typename AreaRasterType::value_type;

    if (size(input) != size(output)) {
        throw InvalidArgument("choropleth avg: raster sizes must match {} vs {}", size(input), size(output));
    }

    std::unordered_map<TAreas, TRasterOutput> sums;
    gdx::for_each_data_value(input, areas, [&sums](TRasterInput value, TAreas area) {
        sums[area] += static_cast<TRasterOutput>(value);
    });

    std::unordered_map<TAreas, int64_t> counts;
    gdx::for_each_data_value(areas, [&counts](TAreas area) {
        ++counts[area];
    });

    gdx::transform(areas, output, [&sums, &counts](TAreas value) {
        auto count = static_cast<double>(counts[value]);
        if (count == 0.0) {
            return static_cast<TRasterOutput>(0.0);
        }
        return static_cast<TRasterOutput>(sums[value] / count);
    });
}
}
