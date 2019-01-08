#pragma once

#include "gdx/exception.h"

#include <algorithm>
#include <gsl/span>
#include <limits>
#include <sstream>

namespace gdx {

template <uint32_t HistogramValues>
struct RasterStats
{
    size_t negativeValues = 0;
    size_t countHigh      = 0;
    size_t nonZeroValues  = 0;
    size_t zeroValues     = 0;
    size_t nanValues      = 0;
    size_t noDataValues   = 0;

    double sum          = 0.0;
    double sigmaNonZero = 0.0;
    double highestValue = std::numeric_limits<double>::lowest();
    double lowestValue  = std::numeric_limits<double>::max();

    std::array<size_t, HistogramValues> histogram = {};

    std::string toString() const
    {
        std::stringstream ss;

        ss << "Min: " << lowestValue
           << " Max: " << highestValue
           << " Nodata: " << noDataValues
           << " Zero: " << zeroValues
           << " Nonzero: " << nonZeroValues
           << " Negative: " << negativeValues
           << " NaN: " << nanValues
           << " Sigma non zero: " << sigmaNonZero
           << " Sum: " << sum;

        return ss.str();
    }
};

template <typename RasterType, uint32_t HistogramValues = 1024>
RasterStats<HistogramValues> statistics(const RasterType& ras, double maxValue)
{
    RasterStats<HistogramValues> stats;
    if (maxValue >= HistogramValues) {
        maxValue = HistogramValues - 1;
    }

    if (maxValue < 0) {
        maxValue = 0;
    }

    for (int i = 0; i < ras.size(); ++i) {
        if (ras.is_nodata(i)) {
            ++stats.noDataValues;
            continue;
        }

        auto value = ras[i];
        if constexpr (std::decay_t<RasterType>::raster_type_has_nan) {
            if (std::isnan(value)) {
                ++stats.nanValues;
                continue;
            }
        }

        if (value != 0) {
            ++stats.nonZeroValues;
        } else {
            ++stats.zeroValues;
        }

        stats.sum += value;
        stats.highestValue = std::max(stats.highestValue, static_cast<double>(value));
        stats.lowestValue  = std::min(stats.lowestValue, static_cast<double>(value));

        int32_t intValue = static_cast<int>(value);

        if (intValue < 0) {
            ++stats.negativeValues;
        } else if (maxValue < value) {
            ++stats.countHigh;
        } else {
            ++stats.histogram[intValue];
        }

        if (value != 0) {
            // calculate sigma via sample variance, see https://en.wikipedia.org/wiki/variance#Sample_variance
            stats.sigmaNonZero += value * value;
        }
    }

    size_t n     = stats.nonZeroValues;
    double mu    = stats.sum / n;
    double sigma = std::sqrt((stats.sigmaNonZero / n) - (mu * mu));
    sigma /= sqrt((n - 1.0) / static_cast<double>(n)); // see page https://en.wikipedia.org/wiki/variance#Sample_variance
    stats.sigmaNonZero = sigma;

    return stats;
}
}
