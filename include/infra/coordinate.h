#pragma once

#include <cinttypes>
#include <fmt/core.h>
#include <limits>

namespace inf {

// Represents a point in the raster using x,y coordinates
struct Coordinate
{
    constexpr Coordinate() = default;
    constexpr Coordinate(double lon, double lat) noexcept
    : longitude(lon)
    , latitude(lat)
    {
    }

    constexpr bool operator==(const Coordinate& other) const
    {
        return longitude == other.longitude && latitude == other.latitude;
    }

    constexpr bool operator!=(const Coordinate& other) const
    {
        return !(*this == other);
    }

    constexpr bool is_valid() const
    {
        return longitude != std::numeric_limits<double>::max() && latitude != std::numeric_limits<double>::max();
    }

    double longitude = std::numeric_limits<double>::max();
    double latitude  = std::numeric_limits<double>::max();
};

constexpr Coordinate operator-(const Coordinate& lhs, const Coordinate& rhs)
{
    return Coordinate(lhs.longitude - rhs.longitude, lhs.latitude - rhs.latitude);
}

constexpr Coordinate operator+(const Coordinate& lhs, const Coordinate& rhs)
{
    return Coordinate(lhs.longitude + rhs.longitude, lhs.latitude + rhs.latitude);
}

}

namespace fmt {
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
        return format_to(ctx.begin(), "(lng:{}, lat:{})", p.longitude, p.latitude);
    }
};
}
