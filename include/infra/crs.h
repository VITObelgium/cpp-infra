#pragma once

#include <cinttypes>
#include <cmath>
#include <fmt/core.h>
#include <limits>

namespace inf::crs {

namespace epsg {
// Epsg integer values
//! Web mercator (typically used as warp target, when displaying rasters on top of tile data)
static inline constexpr int32_t WGS84Spherical   = 3857;
static inline constexpr int32_t WGS84Projected   = 4326;
static inline constexpr int32_t BelgianLambert72 = 31370;
}

/*! Use this enum when you care about type safety in your interfaces */
enum class Epsg : int32_t
{
    WGS84Spherical   = epsg::WGS84Spherical,
    WGS84Projected   = epsg::WGS84Projected,
    BelgianLambert72 = epsg::BelgianLambert72,
};

}
