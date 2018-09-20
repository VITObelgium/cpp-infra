#pragma once

#include <cinttypes>
#include <fmt/core.h>

namespace inf {

struct Color
{
    Color() = default;
    Color(uint8_t r_, uint8_t g_, uint8_t b_) noexcept;
    Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) noexcept;

    bool operator==(const Color& other) const noexcept;

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
        return format_to(ctx.begin(), "({}, {}, {}, {})", int(color.r), int(color.g), int(color.b), int(color.a));
    }
};
}
