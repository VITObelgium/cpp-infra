#pragma once

#include "infra/coordinate.h"
#include "infra/point.h"

namespace inf {

template <typename T>
Coordinate to_coordinate(Point<T> point)
{
    return Coordinate(point.y, point.x);
}

}
