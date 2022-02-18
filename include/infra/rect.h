#pragma once

#include "infra/point.h"

namespace inf {

template <typename T>
struct Rect
{
    Point<T> topLeft;
    Point<T> bottomRight;

    T width() const noexcept
    {
        return std::abs(bottomRight.x - topLeft.x);
    }

    T height() const noexcept
    {
        return std::abs(bottomRight.y - topLeft.y);
    }

    bool is_valid() const noexcept
    {
        return topLeft.is_valid() && bottomRight.is_valid();
    }

    Point<T> top_left() const noexcept
    {
        return topLeft;
    }

    Point<T> top_right() const noexcept
    {
        return Point(bottomRight.x, topLeft.y);
    }

    Point<T> bottom_left() const noexcept
    {
        return Point(topLeft.x, bottomRight.y);
    }

    Point<T> bottom_right() const noexcept
    {
        return bottomRight;
    }
};

template <typename T>
Rect<T> rectangle_intersection(const Rect<T>& lhs, const Rect<T>& rhs) noexcept
{
    auto topLeft     = Point<T>(std::max(lhs.topLeft.x, rhs.topLeft.x), std::min(lhs.topLeft.y, rhs.topLeft.y));
    auto bottomRight = Point<T>(std::min(lhs.bottomRight.x, rhs.bottomRight.x), std::max(lhs.bottomRight.y, rhs.bottomRight.y));
    return {topLeft, bottomRight};
}
}
