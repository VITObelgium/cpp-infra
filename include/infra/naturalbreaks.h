#pragma once

#include "infra/cast.h"
#include "infra/progressinfo.h"
#include "infra/span.h"

#include <cstdint>
#include <limits>
#include <vector>
#include <algorithm>

#ifdef INFRA_TBB
#include <tbb/parallel_for.h>
#endif

namespace inf {

template <typename T>
std::vector<T> jenks_break_values(std::span<T> values, uint8_t numClasses, const inf::ProgressInfo::Callback& progressCb = nullptr)
{
    // Sort the target array
    std::sort(values.begin(), values.end());
    return jenks_break_values(std::span<const T>(values), numClasses, progressCb);
}

template <typename T>
std::vector<T> jenks_break_values(std::span<const T> values, uint8_t numClasses, const inf::ProgressInfo::Callback& progressCb = nullptr)
{
    const auto length_array = values.size();
    std::vector<T> breaks(numClasses + 1, 0.0);

    std::vector<std::vector<int>> lower_class_limits(length_array);
    std::vector<std::vector<T>> variance_combinations(length_array);

    // Initialise the lower_class_limits and variance_combinations arrays
    for (auto& class_limits : lower_class_limits) {
        class_limits.resize(numClasses, 1);
    }

    for (auto& var : variance_combinations) {
        var.resize(numClasses, T(0));
    }

    for (size_t i = 1; i < length_array; ++i) {
        for (size_t j = 0; j < numClasses; ++j) {
            variance_combinations[i][j] = std::numeric_limits<T>::max();
        }
    }

    inf::ProgressInfo progress(length_array - 1, progressCb);

#ifdef INFRA_TBB
    tbb::parallel_for(tbb::blocked_range<size_t>(2, length_array + 1), [&](tbb::blocked_range<size_t> r) {
        for (size_t l = r.begin(); l < r.end(); ++l) {
#else
    for (size_t l = 2; l < length_array + 1; l++) {
#endif
            progress.tick_throw_on_cancel();

            T sum         = 0.0;
            T sum_squares = 0.0;
            T w           = 0.0;
            T variance    = 0.0;

            for (size_t m = 1; m < l + 1; m++) {
                size_t lower_class_limit = l - m + 1;

                T val = values[lower_class_limit - 1];

                w += 1.0;
                sum += val;
                sum_squares += val * val;
                variance  = sum_squares - (sum * sum) / w;
                size_t i4 = lower_class_limit - 1;

                if (i4 != 0) {
                    for (uint8_t j = 2; j < numClasses + 1; ++j) {
                        T temp_val = (variance + variance_combinations[i4 - 1][j - 2]);
                        if (fabs(variance_combinations[l - 1][j - 1] - temp_val) < std::numeric_limits<T>::epsilon() || variance_combinations[l - 1][j - 1] > temp_val) {
                            lower_class_limits[l - 1][j - 1]    = inf::truncate<int>(lower_class_limit);
                            variance_combinations[l - 1][j - 1] = temp_val;
                        }
                    }
                }
            }

            lower_class_limits[l - 1][0]    = 1;
            variance_combinations[l - 1][0] = variance;
        }
#ifdef INFRA_TBB
    });
#endif

    // Prepare the class limits
    size_t k = length_array;
    // First value is the minimum of the target array
    breaks[0] = values[0];
    // Last value is the maximum of the target array
    breaks[numClasses] = values[length_array - 1];

    for (size_t j = numClasses; j > 1; j--) {
        breaks[j - 1] = values[lower_class_limits[k - 1][j - 1] - 2];
        k             = lower_class_limits[k - 1][j - 1] - 1;
    }

    return breaks;
}

template <uint8_t NumClasses, typename T>
std::vector<T> jenks_break_values(std::span<T> values, const inf::ProgressInfo::Callback& progressCb = nullptr)
{
    const auto length_array = values.size();
    std::vector<T> breaks(NumClasses + 1, 0.0);

    std::vector<std::array<int, NumClasses>> lower_class_limits(length_array);
    std::vector<std::array<T, NumClasses>> variance_combinations(length_array);

    // Sort the target array
    std::sort(values.begin(), values.end());

    // Initialise the lower_class_limits and variance_combinations arrays
    for (auto& class_limits : lower_class_limits) {
        class_limits.fill(1);
    }

    for (auto& var : variance_combinations) {
        var.fill(T(0));
    }

    for (size_t i = 1; i < length_array; ++i) {
        for (size_t j = 0; j < NumClasses; ++j) {
            variance_combinations[i][j] = std::numeric_limits<T>::max();
        }
    }

    inf::ProgressInfo progress(length_array - 1, progressCb);

#ifdef INFRA_TBB
    tbb::parallel_for(tbb::blocked_range<size_t>(2, length_array + 1), [&](tbb::blocked_range<size_t> r) {
        for (size_t l = r.begin(); l < r.end(); ++l) {
#else
    for (size_t l = 2; l < length_array + 1; l++) {
#endif
            progress.tick_throw_on_cancel();

            T sum         = 0.0;
            T sum_squares = 0.0;
            T w           = 0.0;
            T variance    = 0.0;

            for (size_t m = 1; m < l + 1; m++) {
                size_t lower_class_limit = l - m + 1;

                T val = values[lower_class_limit - 1];

                w += 1.0;
                sum += val;
                sum_squares += val * val;
                variance  = sum_squares - (sum * sum) / w;
                size_t i4 = lower_class_limit - 1;

                if (i4 != 0) {
                    for (size_t j = 2; j < NumClasses + 1; ++j) {
                        T temp_val = (variance + variance_combinations[i4 - 1][j - 2]);
                        if (fabs(variance_combinations[l - 1][j - 1] - temp_val) < std::numeric_limits<T>::epsilon() || variance_combinations[l - 1][j - 1] > temp_val) {
                            lower_class_limits[l - 1][j - 1]    = inf::truncate<int>(lower_class_limit);
                            variance_combinations[l - 1][j - 1] = temp_val;
                        }
                    }
                }
            }

            lower_class_limits[l - 1][0]    = 1;
            variance_combinations[l - 1][0] = variance;
        }
#ifdef INFRA_TBB
    });
#endif

    // Prepare the class limits
    size_t k = length_array;
    // First value is the minimum of the target array
    breaks[0] = values[0];
    // Last value is the maximum of the target array
    breaks[NumClasses] = values[length_array - 1];

    for (size_t j = NumClasses; j > 1; j--) {
        breaks[j - 1] = values[lower_class_limits[k - 1][j - 1] - 2];
        k             = lower_class_limits[k - 1][j - 1] - 1;
    }

    return breaks;
}
}
