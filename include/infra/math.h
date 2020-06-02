#pragma once

#include <math.h>

namespace inf::math {

static inline constexpr double pi = 3.14159265358979323846;

constexpr double rad_to_deg(double rad)
{
    return rad * 180.0 / pi;
}

constexpr double deg_to_rad(double deg)
{
    return deg * pi / 180.0;
}

}
