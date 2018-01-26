#pragma once

#include <type_traits>

namespace infra {

template <typename TDest, typename TSrc>
TDest truncate(TSrc value)
{
    static_assert(std::is_arithmetic_v<TSrc>, "Only use truncate on arithmetic types");
    return static_cast<TDest>(value);
}
}
