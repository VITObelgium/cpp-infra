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
};

template <typename T>
Rect<T> rectangle_intersection(const Rect<T>& lhs, const Rect<T>& rhs) noexcept
{
    auto topLeft     = Point<double>(std::max(lhs.topLeft.x, rhs.topLeft.x), std::min(lhs.topLeft.y, rhs.topLeft.y));
    auto bottomRight = Point<double>(std::min(lhs.bottomRight.x, rhs.bottomRight.x), std::max(lhs.bottomRight.y, rhs.bottomRight.y));
    return {topLeft, bottomRight};
}
}
