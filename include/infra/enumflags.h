#pragma once

#include <type_traits>

namespace infra {

template <typename EnumType>
class Flags
{
public:
    using value_type = std::underlying_type_t<EnumType>;

    constexpr Flags() = default;

    constexpr Flags(EnumType e)
    : _value(static_cast<value_type>(e))
    {
    }

    constexpr Flags(const Flags& f) = default;
    constexpr Flags(Flags&& f)      = default;

    constexpr Flags operator|(EnumType v) const noexcept
    {
        Flags f;
        f._value = _value | static_cast<value_type>(v);
        return f;
    }

    constexpr Flags operator|(Flags other) const noexcept
    {
        Flags f;
        f._value = _value | static_cast<value_type>(other._value);
        return f;
    }

    constexpr Flags& operator|=(EnumType v) noexcept
    {
        _value |= static_cast<value_type>(v);
        return *this;
    }

    constexpr Flags& operator|=(Flags& other) noexcept
    {
        _value |= static_cast<value_type>(other._value);
        return *this;
    }

    constexpr bool is_set(EnumType v) const noexcept
    {
        return (static_cast<value_type>(v) & _value) != 0;
    }

private:
    value_type _value = value_type(0);
};

template <typename EnumType, typename = std::enable_if<std::is_enum_v<EnumType>>>
constexpr Flags<EnumType> operator|(EnumType lhs, EnumType rhs)
{
    return Flags<EnumType>() | lhs | rhs;
}
}
