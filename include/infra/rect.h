#pragma once

#include "infra/point.h"

namespace inf {

template <typename T>
struct Rect
{
    Point<T> topLeft;
    Point<T> bottomRight;

    Rect() noexcept = default;

    Rect(Point<T> topLeft_, Point<T> bottomRight_) noexcept
    : topLeft(topLeft_)
    , bottomRight(bottomRight_)
    {
    }

    T width() const noexcept
    {
        if (bottomRight.x > topLeft.x) {
            return bottomRight.x - topLeft.x;
        }

        return 0;
    }

    T height() const noexcept
    {
        return std::abs(bottomRight.y - topLeft.y);
    }

    bool empty() const noexcept
    {
        return width() == 0 || height() == 0;
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

    bool operator==(const Rect<T>& other) const noexcept
    {
        return topLeft == other.topLeft && bottomRight == other.bottomRight;
    }

    bool operator!=(const Rect<T>& other) const noexcept
    {
        return !(*this == other);
    }
};

template <typename T>
bool rectangles_intersect(const Rect<T>& lhs, const Rect<T>& rhs) noexcept
{
    return (lhs.topLeft.x < rhs.bottomRight.x &&
            lhs.bottomRight.x > rhs.topLeft.x &&
            lhs.topLeft.y > rhs.bottomRight.y &&
            lhs.bottomRight.y < rhs.topLeft.y);
}

template <typename T>
Rect<T> rectangle_intersection(const Rect<T>& lhs, const Rect<T>& rhs) noexcept
{
    auto topLeft     = Point<T>(std::max(lhs.topLeft.x, rhs.topLeft.x), std::min(lhs.topLeft.y, rhs.topLeft.y));
    auto bottomRight = Point<T>(std::min(lhs.bottomRight.x, rhs.bottomRight.x), std::max(lhs.bottomRight.y, rhs.bottomRight.y));
    return {topLeft, bottomRight};
}
}
