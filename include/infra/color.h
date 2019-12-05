#pragma once

#include <cinttypes>
#include <fmt/core.h>

namespace inf {

struct Color
{
    constexpr Color() = default;

    constexpr Color(uint8_t r_, uint8_t g_, uint8_t b_) noexcept
    : Color(r_, g_, b_, 255)
    {
    }

    Color(std::string_view hexString);

    constexpr Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) noexcept
    : r(r_)
    , g(g_)
    , b(b_)
    , a(a_)
    {
    }

    constexpr bool operator==(const Color& other) const noexcept
    {
        return r == other.r &&
               g == other.g &&
               b == other.b &&
               a == other.a;
    }

    std::string to_hex_rgb() const noexcept;
    std::string to_hex_argb() const noexcept;

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
};
}

namespace fmt {
template <>
struct formatter<inf::Color>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const inf::Color& color, FormatContext& ctx)
    {
        return format_to(ctx.out(), "({}, {}, {}, {})", int(color.r), int(color.g), int(color.b), int(color.a));
    }
};
}
