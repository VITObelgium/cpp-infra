#pragma once

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

namespace fmt {
template <>
struct formatter<inf::Coordinate>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const inf::Coordinate& p, FormatContext& ctx) const -> decltype(ctx.out())
    {
        return format_to(ctx.out(), "(lat:{:.6f}, lng:{:.6f})", p.latitude, p.longitude);
    }
};
}
