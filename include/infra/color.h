#pragma once

#include <cinttypes>
#include <fmt/core.h>

namespace inf {

struct Color
{
    constexpr Color() = default;

    constexpr Color(uint8_t r, uint8_t g, uint8_t b) noexcept
    : Color(r, g, b, 255)
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

    constexpr bool operator!=(const Color& other) const noexcept
    {
        return !(*this == other);
    }

    std::string to_hex_rgb() const noexcept;
    std::string to_hex_argb() const noexcept;

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
};

namespace color {
static constexpr Color Black(0, 0, 0);
static constexpr Color White(255, 255, 255);
static constexpr Color Blue(0, 0, 255);
static constexpr Color Green(0, 255, 0);
static constexpr Color Red(255, 0, 0);
static constexpr Color Transparent(0, 0, 0, 0);
static constexpr Color DarkGrey(107, 110, 119);
static constexpr Color LightGrey(170, 174, 177);
}

}

template <>
struct fmt::formatter<inf::Color>
{
    FMT_CONSTEXPR20 auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    auto format(const inf::Color& color, format_context& ctx) const -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "({}, {}, {}, {})", int(color.r), int(color.g), int(color.b), int(color.a));
    }
};
