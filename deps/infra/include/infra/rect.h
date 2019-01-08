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
}
