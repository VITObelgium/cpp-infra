#pragma once

#include "infra/point.h"

namespace inf {

template <typename T>
struct Line
{
    Line() = default;
    Line(const Point<T>& startPoint, const Point<T>& endPoint) noexcept
    : start(startPoint)
    , end(endPoint)
    {
    }

    Line(double x0, double y0, double x1, double y1) noexcept
    : start(x0, y0)
    , end(x1, y1)
    {
    }

    Point<T> start;
    Point<T> end;
};
}
