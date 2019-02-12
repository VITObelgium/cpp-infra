#pragma once

#include <cinttypes>
#include <fmt/core.h>
#include <limits>

namespace inf {

// Represents a point in the raster using x,y coordinates
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

    constexpr bool is_valid() const
    {
        return latitude != std::numeric_limits<double>::max() && longitude != std::numeric_limits<double>::max();
    }

    double latitude  = std::numeric_limits<double>::max();
    double longitude = std::numeric_limits<double>::max();
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
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const inf::Coordinate& p, FormatContext& ctx)
    {
        return format_to(ctx.begin(), "(lat:{:.6f}, lng:{:.6f})", p.latitude, p.longitude);
    }
};
}
