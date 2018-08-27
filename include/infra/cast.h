#pragma once

#include <optional>
#include <type_traits>

namespace inf {

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
