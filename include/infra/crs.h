#pragma once

#include "infra/coordinate.h"
#include "infra/geoconstants.h"
#include "infra/math.h"
#include "infra/point.h"

#include <cinttypes>
#include <cmath>
#include <fmt/core.h>
#include <limits>

namespace inf::crs {

namespace epsg {
// Epsg integer values
//! Web mercator (typically used as warp target, when displaying rasters on top of tile data)
static inline constexpr int32_t WGS84WebMercator   = 3857;
static inline constexpr int32_t WGS84              = 4326; // geographic projection
static inline constexpr int32_t BelgianLambert72   = 31370;
static inline constexpr int32_t Belge72Geo         = 4313; // geographic projection
static inline constexpr int32_t ETRS89             = 3035;
static inline constexpr int32_t BelgianLambert2008 = 3812;

}

/*! Use this enum when you care about type safety in your interfaces */
enum class Epsg : int32_t
{
    WGS84WebMercator   = epsg::WGS84WebMercator,
    WGS84              = epsg::WGS84,
    BelgianLambert72   = epsg::BelgianLambert72,
    Belge72Geo         = epsg::Belge72Geo,
    ETRS89             = epsg::ETRS89,
    BelgianLambert2008 = epsg::BelgianLambert2008,
};

// Convert longitude and latitude to web mercator x, y EPSG:4326
inline inf::Point<double> lat_lon_to_web_mercator(inf::Coordinate coord) noexcept
{
    inf::Point<double> result;
    result.x = constants::EARTH_RADIUS_M * math::deg_to_rad(coord.longitude);

    if (coord.latitude <= -90.0) {
        result.y = -std::numeric_limits<double>::infinity();
    } else if (coord.latitude >= 90.0) {
        result.y = std::numeric_limits<double>::infinity();
    } else {
        result.y = constants::EARTH_RADIUS_M * std::log(std::tan((math::pi * 0.25) + (0.5 * math::deg_to_rad(coord.latitude))));
    }

    return result;
}

inline inf::Coordinate web_mercator_to_lat_lon(inf::Point<double> point) noexcept
{
    double latitude  = math::rad_to_deg(2 * std::atan(std::exp(point.y / constants::EARTH_RADIUS_M)) - (math::pi / 2.0));
    double longitude = math::rad_to_deg(point.x) / constants::EARTH_RADIUS_M;

    inf::Coordinate result;
    result.latitude  = std::clamp(latitude, -constants::LATITUDE_MAX, constants::LATITUDE_MAX);
    result.longitude = std::clamp(longitude, -constants::LONGITUDE_MAX, constants::LONGITUDE_MAX);
    return result;
}

}
