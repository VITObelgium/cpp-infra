#pragma once

#include <cinttypes>
#include <fmt/core.h>
#include <limits>

namespace inf {

struct Size
{
    constexpr Size() = default;
    constexpr Size(int32_t width_, int32_t height_) noexcept
    : width(width_), height(height_)
    {
    }

    constexpr bool operator==(const Size& other) const
    {
        return width == other.width && height == other.height;
    }

    constexpr bool operator!=(const Size& other) const
    {
        return !(*this == other);
    }

    constexpr bool is_valid() const
    {
        return width >= 0 && height >= 0;
    }

    int32_t width  = -1;
    int32_t height = -1;
};

}

template <>
struct fmt::formatter<inf::Size>
{
    FMT_CONSTEXPR20 auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    auto format(const inf::Size& s, format_context& ctx) const -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "({}x{})", s.width, s.height);
    }
};

