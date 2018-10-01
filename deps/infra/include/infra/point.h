#pragma once

#include <cinttypes>
#include <fmt/core.h>
#include <limits>

namespace inf {

// Represents a point in the raster using x,y coordinates
template <typename T>
struct Point
{
    Point() = default;
    Point(T x_, T y_) noexcept
    : x(x_), y(y_)
    {
    }

    bool operator==(const Point<T>& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point<T>& other) const
    {
        return !(*this == other);
    }

    T x = std::numeric_limits<T>::max();
    T y = std::numeric_limits<T>::max();
};

}

namespace fmt {
template <typename T>
struct formatter<inf::Point<T>>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const inf::Point<T>& p, FormatContext& ctx)
    {
        if constexpr (std::is_floating_point_v<T>) {
            return format_to(ctx.begin(), "({:.1f}, {:.1f})", p.x, p.y);
        } else {
            return format_to(ctx.begin(), "({}, {})", p.x, p.y);
        }
    }
};
}
