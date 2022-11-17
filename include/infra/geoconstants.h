#pragma once

#include "infra/math.h"

#include <cstdint>

namespace inf::constants {

constexpr double EARTH_RADIUS_M        = 6378137.0;
constexpr double EARTH_CIRCUMFERENCE_M = 2 * math::pi * EARTH_RADIUS_M;
constexpr double LATITUDE_MAX          = 85.051128779806604;
constexpr double LONGITUDE_MAX         = 180;

}
