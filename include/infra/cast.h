#pragma once

#include <optional>
#include <type_traits>

namespace inf {

template <typename T>
bool fitsInType(double value)
{
    if (value >= static_cast<double>(std::numeric_limits<T>::lowest()) &&
        value <= static_cast<double>(std::numeric_limits<T>::max())) {
        return true;
    }

    return false;
}

template <typename TDest, typename TSrc>
TDest truncate(TSrc value)
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
