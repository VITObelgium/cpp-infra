#pragma once

#include <type_traits>

namespace inf {

template <typename EnumType>
constexpr auto enum_value(EnumType e)
{
    return static_cast<typename std::underlying_type_t<EnumType>>(e);
}

// Calling this function only makes sense for enums that use the convention
// of adding a last enum field with the name EnumCount and where the other
// enum fields do not explicitely set their value
template <typename EnumType>
constexpr std::underlying_type_t<EnumType> enum_count()
{
    return enum_value(EnumType::EnumCount);
}
}
