#pragma once

#include <type_traits>

namespace inf {

template <typename EnumType>
class Flags
{
public:
    using value_type = std::underlying_type_t<EnumType>;

    static constexpr Flags from_value(value_type v) noexcept
    {
        return Flags(v);
    }

    constexpr Flags() = default;

    constexpr Flags(EnumType e)
    : _value(static_cast<value_type>(e))
    {
    }

    constexpr Flags(EnumType flag1, EnumType flag2)
    : _value(static_cast<value_type>(flag1) | static_cast<value_type>(flag2))
    {
    }

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

    constexpr Flags operator&(EnumType v) const noexcept
    {
        Flags f;
        f._value = _value & static_cast<value_type>(v);
        return f;
    }

    constexpr Flags operator&(Flags other) const noexcept
    {
        Flags f;
        f._value = _value & static_cast<value_type>(other._value);
        return f;
    }

    constexpr Flags& operator|=(EnumType v) noexcept
    {
        _value |= static_cast<value_type>(v);
        return *this;
    }

    constexpr Flags& operator|=(Flags other) noexcept
    {
        _value |= static_cast<value_type>(other._value);
        return *this;
    }

    constexpr bool operator==(Flags other) const noexcept
    {
        return _value == other._value;
    }

    constexpr bool operator!=(Flags other) const noexcept
    {
        return !(*this == other);
    }

    constexpr bool is_set(EnumType v) const noexcept
    {
        return (static_cast<value_type>(v) & _value) != 0;
    }

    constexpr void unset(EnumType v) noexcept
    {
        _value &= ~(static_cast<value_type>(v));
    }

    constexpr void set(EnumType v) noexcept
    {
        _value |= static_cast<value_type>(v);
    }

    constexpr explicit operator value_type() const noexcept
    {
        return _value;
    }

    constexpr bool empty() const noexcept
    {
        return _value == 0;
    }

private:
    constexpr Flags(value_type v)
    : _value(static_cast<value_type>(v))
    {
    }

    value_type _value = value_type(0);
};

}
