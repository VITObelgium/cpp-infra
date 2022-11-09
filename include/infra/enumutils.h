#pragma once

#include <array>
#include <type_traits>

namespace inf {

template <typename EnumType>
constexpr auto enum_value(EnumType e) noexcept
{
    return static_cast<typename std::underlying_type_t<EnumType>>(e);
}

template <typename EnumType>
using enum_type = std::underlying_type<EnumType>;

template <typename EnumType>
using enum_type_t = std::underlying_type_t<EnumType>;

// Calling this function only makes sense for enums that use the convention
// of adding a last enum field with the name EnumCount and where the other
// enum fields do not explicitely set their value
template <typename EnumType>
constexpr std::underlying_type_t<EnumType> enum_count() noexcept
{
    return enum_value(EnumType::EnumCount);
}

template <typename EnumType>
constexpr std::array<EnumType, enum_count<EnumType>()> enum_entries() noexcept
{
    std::array<EnumType, enum_count<EnumType>()> result;

    for (std::underlying_type_t<EnumType> i = 0; i < enum_count<EnumType>(); ++i) {
        result[i] = EnumType(i);
    }

    return result;
}

}
