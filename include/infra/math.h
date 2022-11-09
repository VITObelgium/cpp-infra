#pragma once

#include "infra/cast.h"
#include "infra/span.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <type_traits>
#include <vector>


namespace inf::math {

static inline constexpr double pi = 3.14159265358979323846;

template <typename T>
constexpr T nan()
{
    static_assert(std::is_floating_point_v<T>, "nan should be called on floating point type");
    return std::numeric_limits<T>::quiet_NaN();
}

template <typename T>
constexpr T rad_to_deg(T rad)
{
    return static_cast<T>(rad * 180.0 / pi);
}

template <typename T>
constexpr T deg_to_rad(T deg)
{
    return truncate<T>(deg * pi / 180.0);
}

/*! Calculate the percentile of the value range
 * Will modify the values order by sorting them to determine the percentile
 * /param percentile the percentile (e.g. 10 to get the 10th percentile)
 * /param values sorted input values, the result will be incorrect if these values are not sorted
 * /return the percentile
 */
template <typename T>
T percentile_sorted_input(double percentile, std::span<const T> values)
{
    auto rank    = percentile / 100 * (values.size() + 1);
    size_t index = std::clamp<long long>(std::llround(rank - 1.0), 0, values.size() - 1);
    return values[index];
}

/*! Calculate the percentile of the value range
 * Will modify the values order by sorting them to determine the percentile
 * /param percentile the percentile (e.g. 10 to get the 10th percentile)
 * /param values input values, these will get sorted during the calcalation
 * /return the percentile
 */
template <typename T>
T percentile_in_place(double percentile, std::span<T> values)
{
    std::sort(values.begin(), values.end());
    return percentile_sorted_input<T>(percentile, values);
}

/*! Calculate the percentile of the value range
 * /param percentile the percentile (e.g. 10 to get the 10th percentile)
 * /param values input values
 * /return the percentile
 */
template <typename T>
T percentile(double percentile, std::span<const T> values)
{
    std::vector<T> dataCopy(values.begin(), values.end());
    return percentile_in_place<T>(percentile, dataCopy);
}

/*! Calculate the mean value
 * /param values input values
 * /return the mean
 */
template <typename T>
double mean(std::span<const T> values)
{
    auto sum = std::accumulate(values.begin(), values.end(), T());
    return sum / double(values.size());
}

/*! Calculate the standard deviation
 * /param values input values
 * /return the standard deviation
 */
template <typename T>
double standard_deviation(std::span<const T> values)
{
    double mean   = math::mean<T>(values);
    double stdDev = 0.0;

    for (auto& val : values) {
        stdDev += std::pow(val - mean, 2);
    }

    return std::sqrt(stdDev / values.size());
}

template <typename T>
bool approx_equal(const T& lhs, const T& rhs, T epsilon = std::numeric_limits<T>::epsilon())
{
    return std::abs(lhs - rhs) <= epsilon;
}

template <typename T>
bool approx_equal_opt(const std::optional<T>& lhs, const std::optional<T>& rhs, T epsilon = std::numeric_limits<T>::epsilon())
{
    if (lhs.has_value() && rhs.has_value()) {
        return approx_equal(*lhs, *rhs, epsilon);
    }

    return lhs.has_value() == rhs.has_value();
}

template <typename T>
inline T floor_to_neirest_multiple(T val, int32_t base) noexcept
{
    T mod = val % base;
    val -= mod;
    return val;
}

template <typename T>
inline T ceil_to_neirest_multiple(T val, int32_t base) noexcept
{
    T mod = val % base;
    val += base - mod;
    return val;
}

}
