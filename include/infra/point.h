#pragma once

#include <cinttypes>
#include <fmt/core.h>
#include <limits>

namespace inf {

// Represents a point in the raster using x,y coordinates
template <typename T>
struct Point
{
    constexpr Point() = default;
    constexpr Point(T x_, T y_) noexcept
    : x(x_), y(y_)
    {
    }

    constexpr bool operator==(const Point<T>& other) const
    {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Point<T>& other) const
    {
        return !(*this == other);
    }

    constexpr bool is_valid() const
    {
        return x != std::numeric_limits<T>::max() && y != std::numeric_limits<T>::max();
    }

    T x = std::numeric_limits<T>::max();
    T y = std::numeric_limits<T>::max();
};

template <typename T>
constexpr Point<T> operator-(const Point<T>& lhs, const Point<T>& rhs)
{
    return Point<T>(lhs.x - rhs.x, lhs.y - rhs.y);
}

template <typename T>
constexpr Point<T> operator+(const Point<T>& lhs, const Point<T>& rhs)
{
    return Point<T>(lhs.x + rhs.x, lhs.y + rhs.y);
}

template <typename TTo, typename TFrom>
constexpr Point<TTo> point_cast(const Point<TFrom>& from)
{
    return Point<TTo>(static_cast<TTo>(from.x), static_cast<TTo>(from.y));
}

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
