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

namespace fmt {
template <>
struct formatter<inf::Size>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const inf::Size& s, FormatContext& ctx)
    {
        return format_to(ctx.out(), "({}x{})", s.width, s.height);
    }
};
}
