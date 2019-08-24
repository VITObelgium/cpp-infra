#pragma once

#include <cinttypes>
#include <cmath>
#include <fmt/core.h>
#include <limits>

namespace inf::epsg {

//! Web mercator (typically used as warp target, when displaying rasters on top of tile data)
static inline constexpr int32_t WGS84Spherical   = 3857;
static inline constexpr int32_t WGS84Projected   = 4326;
static inline constexpr int32_t BelgianLambert72 = 31370;

}
