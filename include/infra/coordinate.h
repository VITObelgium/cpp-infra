#pragma once

#include "infra/geoconstants.h"

#include <cinttypes>
#include <cmath>
#include <fmt/core.h>
#include <limits>

namespace inf {

// Represents a wgs84 point in the raster (lat, lon)
struct Coordinate
{
    constexpr Coordinate() = default;
    constexpr Coordinate(double lat, double lon) noexcept
    : latitude(lat)
    , longitude(lon)
    {
    }

    constexpr bool operator==(const Coordinate& other) const
    {
        return latitude == other.latitude && longitude == other.longitude;
    }

    constexpr bool operator!=(const Coordinate& other) const
    {
        return !(*this == other);
    }

    bool is_valid() const noexcept
    {
        if (std::isnan(latitude) || std::isnan(longitude)) {
            return false;
        }

        if (std::abs(latitude) > 90.0) {
            // latitude must be between -90 and 90
            return false;
        }

        if (!std::isfinite(longitude)) {
            return false;
        }

        return true;
    }

    constexpr Coordinate wrapped() const
    {
        Coordinate coord(latitude, longitude);
        coord.wrap();
        return coord;
    }

    constexpr void wrap()
    {
        longitude = wrap(longitude, -constants::LONGITUDE_MAX, constants::LONGITUDE_MAX);
    }

private:
    // Constrains n to the given range (including min, excluding max) via modular arithmetic.
    constexpr double wrap(double value, double min, double max)
    {
        if (value >= min && value < max) {
            return value;
        } else if (value == max) {
            return min;
        }

        const double delta   = max - min;
        const double wrapped = min + std::fmod(value - min, delta);
        return value < min ? wrapped + delta : wrapped;
    }

public:
    double latitude  = std::numeric_limits<double>::quiet_NaN();
    double longitude = std::numeric_limits<double>::quiet_NaN();
};

inline double distance(const Coordinate& lhs, const Coordinate& rhs)
{
    auto lat = rhs.latitude - lhs.latitude;
    auto lon = rhs.longitude - lhs.longitude;

    return std::sqrt((lat * lat) + (lon * lon));
}

constexpr Coordinate operator-(const Coordinate& lhs, const Coordinate& rhs)
{
    return Coordinate(lhs.latitude - rhs.latitude, lhs.longitude - rhs.longitude);
}

constexpr Coordinate operator+(const Coordinate& lhs, const Coordinate& rhs)
{
    return Coordinate(lhs.latitude + rhs.latitude, lhs.longitude + rhs.longitude);
}

}

template <>
struct fmt::formatter<inf::Coordinate>
{
    FMT_CONSTEXPR20 auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    auto format(inf::Coordinate p, format_context& ctx) const -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "(lat:{:.6f}, lng:{:.6f})", p.latitude, p.longitude);
    }
};
