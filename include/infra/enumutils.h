#pragma once

#include <type_traits>

namespace infra
{

template <typename EnumType>
constexpr auto enum_value(EnumType e)
{
    return static_cast<typename std::underlying_type_t<EnumType>>(e);
}

}