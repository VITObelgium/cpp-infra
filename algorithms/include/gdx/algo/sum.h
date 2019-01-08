#pragma once

#include "gdx/algo/algorithm.h"
#include "gdx/exception.h"

#include <gsl/span>
#include <numeric>
#include <unordered_map>

namespace gdx {

template <typename RasterType>
double sum(const RasterType& ras)
{
    double sum      = 0.0;
    const auto size = ras.size();

#pragma omp parallel for reduction(+ \
                                   : sum)
    for (int i = 0; i < size; ++i) {
        if (!ras.is_nodata(i)) {
            sum += ras[i];
        }
    }

    return sum;
}

template <typename RasterType>
double ssum(const RasterType& ras)
{
    using T = typename RasterType::value_type;
    return static_cast<double>(std::accumulate(value_begin(ras), value_end(ras), T(0), std::plus<T>()));
}

/*!
 * \brief Apply a mask to the grid and create the sum of all values in the mask.
 * \param ras the input raster
 * \param mask a raster containing mask values
 * \param maskValues an array with mask values for which sums are made
 * \return a unordered_map of value, sum pairs
 */
template <typename SumType, typename RasterType, typename MaskType>
auto sumMask(const RasterType& ras, const MaskType& mask)
{
    using RasterValueType = typename RasterType::value_type;
    using MaskValueType   = typename MaskType::value_type;

    if (size(ras) != size(mask)) {
        throw InvalidArgument("sumMask: raster sizes must match {} vs {}", size(ras), size(mask));
    }

    std::unordered_map<MaskValueType, SumType> result;
    gdx::for_each_data_value(ras, mask, [&result](RasterValueType rasterValue, MaskValueType maskValue) {
        result[maskValue] += static_cast<SumType>(rasterValue);
    });

    return result;
}

/*!
 * \brief Apply a mask to the grid and create the sum of all values in the mask.
 * \param ras the input raster
 * \param mask a raster containing mask values
 * \param maskValues an array with mask values for which sums are made, other values in the mask are ignored
 * \return a unordered_map of value, sum pairs
 */
template <typename ResultType, typename RasterType, typename MaskType>
auto sumMask(const RasterType& ras, const MaskType& mask, gsl::span<const typename MaskType::value_type> maskValues) -> std::unordered_map<typename MaskType::value_type, ResultType>
{
    using RasterValueType = typename RasterType::value_type;
    using MaskValueType   = typename MaskType::value_type;

    std::unordered_map<MaskValueType, ResultType> result;
    for (auto& value : maskValues) {
        result[value] = ResultType(0);
    }

    gdx::for_each_data_value(ras, mask, [&](RasterValueType rasterValue, MaskValueType maskValue) {
        auto iter = result.find(maskValue);
        if (iter != result.end()) {
            result[maskValue] += static_cast<ResultType>(rasterValue);
        }
    });

    return result;
}
}
