#pragma once

#include <cinttypes>
#include <cmath>
#include <fmt/core.h>
#include <limits>

namespace inf::crs {

namespace epsg {
// Epsg integer values
//! Web mercator (typically used as warp target, when displaying rasters on top of tile data)
static inline constexpr int32_t WGS84WebMercator = 3857;
static inline constexpr int32_t WGS84            = 4326; // geographic projection
static inline constexpr int32_t BelgianLambert72 = 31370;
static inline constexpr int32_t Belge72Geo       = 4313; // geographic projection
}

/*! Use this enum when you care about type safety in your interfaces */
enum class Epsg : int32_t
{
    WGS84WebMercator = epsg::WGS84WebMercator,
    WGS84            = epsg::WGS84,
    BelgianLambert72 = epsg::BelgianLambert72,
    Belge72Geo       = epsg::Belge72Geo,
};

}
