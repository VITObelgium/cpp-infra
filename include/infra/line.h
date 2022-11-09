#pragma once

#include "infra/hash.h"
#include "infra/point.h"

namespace inf {

template <typename T>
struct Line
{
    constexpr Line() = default;
    constexpr Line(const Point<T>& startPoint, const Point<T>& endPoint) noexcept
    : start(startPoint)
    , end(endPoint)
    {
    }

    constexpr Line(double x0, double y0, double x1, double y1) noexcept
    : start(x0, y0)
    , end(x1, y1)
    {
    }

    constexpr bool operator==(const Line<T>& other) const noexcept
    {
        return start == other.start && end == other.end;
    }

    Point<T> start;
    Point<T> end;
};
}

namespace std {
    template <typename T>
    struct hash<inf::Line<T>>
    {
        size_t operator()(const inf::Line<T>& line) const
        {
            size_t seed = 0;
            inf::hash_combine(seed, line.start, line.end);
            return seed;
        }
    };
}

