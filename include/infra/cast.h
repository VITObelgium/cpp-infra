#pragma once

#include <cmath>
#include <limits>
#include <optional>
#include <type_traits>

#include <fmt/format.h>

namespace inf {

template <typename T>
struct larger_type
{
    using type = T;
};

// clang-format off
template <> struct larger_type<int8_t>   { using type = int16_t; };
template <> struct larger_type<int16_t>  { using type = int32_t; };
template <> struct larger_type<int32_t>  { using type = int64_t; };
template <> struct larger_type<uint8_t>  { using type = uint16_t; };
template <> struct larger_type<uint16_t> { using type = uint32_t; };
template <> struct larger_type<uint32_t> { using type = uint64_t; };
template <> struct larger_type<float>    { using type = double; };
template <> struct larger_type<double>   { using type = long double; };
// clang-format on

template <typename T>
using larger_type_t = typename larger_type<T>::type;

template <typename TCheck, typename TInput>
constexpr bool fits_in_type(TInput value)
{
    if constexpr (std::is_same_v<TInput, TCheck>) {
        // same type, always fits
        (void)value;
        return true;
    } else if constexpr ((std::is_unsigned_v<TInput> && std::is_unsigned_v<TCheck>) ||
                         (std::is_signed_v<TInput> && std::is_signed_v<TCheck>) ||
                         (std::is_floating_point_v<TInput> && std::is_floating_point_v<TCheck>)) {
        // Both types have the same signedness or floats
        using Common = std::common_type_t<TInput, TCheck>;
        return static_cast<Common>(value) >= static_cast<Common>(std::numeric_limits<TCheck>::lowest()) &&
               static_cast<Common>(value) <= static_cast<Common>(std::numeric_limits<TCheck>::max());
    } else if constexpr (std::is_unsigned_v<TCheck>) {
        // Type to check against is unsigned, so the value must be in range [0, TCheck::max]
        using Common = std::common_type_t<TInput, TCheck>;
        return value >= 0 && static_cast<Common>(value) <= static_cast<Common>(std::numeric_limits<TCheck>::max());
    } else if constexpr (std::is_floating_point_v<TInput> || std::is_floating_point_v<TCheck>) {
        // One of the type is floating point, upcast everything to double
        return static_cast<double>(value) >= static_cast<double>(std::numeric_limits<TCheck>::lowest()) &&
               static_cast<double>(value) <= static_cast<double>(std::numeric_limits<TCheck>::max());
    } else {
        // no floats, signed vs unsigned type
        using Common = std::common_type_t<larger_type_t<TInput>, larger_type_t<TCheck>>;
        return static_cast<Common>(value) >= static_cast<Common>(std::numeric_limits<TCheck>::lowest()) &&
               static_cast<Common>(value) <= static_cast<Common>(std::numeric_limits<TCheck>::max());
    }
}

template <typename TDest, typename TSrc>
constexpr TDest truncate(TSrc value)
{
    static_assert(std::is_arithmetic_v<TSrc>, "Only use truncate on arithmetic types");
    return static_cast<TDest>(value);
}

template <class T, class U>
std::optional<T> optional_cast(const std::optional<U>& u)
{
    if (u.has_value()) {
        return std::make_optional<T>(static_cast<T>(*u));
    } else {
        return std::optional<T>();
    }
}
}
