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

namespace std {
template <typename T>
struct hash<inf::Point<T>>
{
    size_t operator()(const inf::Point<T>& point) const
    {
        size_t h1 = hash<T>()(point.x);
        size_t h2 = hash<T>()(point.y);
        return hash<long long>()((h1 << 32) ^ h2);
    }
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
            return format_to(ctx.out(), "({:.1f}, {:.1f})", p.x, p.y);
        } else {
            return format_to(ctx.out(), "({}, {})", p.x, p.y);
        }
    }
};
}
