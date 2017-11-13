#pragma once

#include <cinttypes>
#include <limits>
#include <ostream>

namespace infra {

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

template <typename T>
std::ostream& operator<<(std::ostream& os, const Point<T>& point)
{
    return os << point.x << "x" << point.y;
}
}
