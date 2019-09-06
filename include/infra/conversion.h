#pragma once

#include "infra/cast.h"
#include "infra/coordinate.h"
#include "infra/point.h"

namespace inf {

template <typename T>
Coordinate to_coordinate(Point<T> point)
{
    return Coordinate(point.y, point.x);
}

template <typename T = double>
Point<T> to_point(Coordinate coord)
{
    return Point<T>(truncate<T>(coord.longitude), truncate<T>(coord.latitude));
}

}
