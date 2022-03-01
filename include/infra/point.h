#pragma once

#include <cinttypes>
#include <cmath>
#include <fmt/core.h>
#include <limits>

namespace inf {

// Represents a point in the raster using x,y coordinates
template <typename T>
struct Point
{
    constexpr Point() noexcept = default;
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
double distance(const Point<T>& lhs, const Point<T>& rhs)
{
    auto x = rhs.x - lhs.x;
    auto y = rhs.y - lhs.y;

    return std::sqrt((x * x) + (y * y));
}

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

template <typename T>
std::ostream& operator<<(std::ostream& os, const Point<T>& p)
{
    os << fmt::format("{}", p);
    return os;
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
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const inf::Point<T>& p, FormatContext& ctx) const -> decltype(ctx.out())
    {
        if constexpr (std::is_floating_point_v<T>) {
            return format_to(ctx.out(), "({:.1f}, {:.1f})", p.x, p.y);
        } else {
            return format_to(ctx.out(), "({}, {})", p.x, p.y);
        }
    }
};
}
