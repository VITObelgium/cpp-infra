#pragma once

#include "infra/coordinate.h"

#include <cassert>
#include <cstdint>

namespace inf {

class LatLonBounds
{
public:
    // Return a bounds covering the entire (unwrapped) world.
    static LatLonBounds world()
    {
        return {{-90, -180}, {90, 180}};
    }

    // Return the convex hull of two points; the smallest bounds that contains both.
    static LatLonBounds hull(const Coordinate& a, const Coordinate& b)
    {
        LatLonBounds bounds(a, a);
        bounds.extend(b);
        return bounds;
    }

    // Return a bounds that may serve as the identity element for the extend operation.
    static LatLonBounds empty()
    {
        LatLonBounds bounds = world();
        std::swap(bounds.sw, bounds.ne);
        return bounds;
    }

    /// Construct an infinite bound, a bound for which the constrain method returns its
    /// input unmodified.
    ///
    /// Note: this is different than LatLonBounds::world() since arbitrary unwrapped
    /// coordinates are also inside the bounds.
    LatLonBounds()
    : sw({-90, -180}), ne({90, 180}), bounded(false)
    {
    }

    bool valid() const noexcept
    {
        return (sw.latitude <= ne.latitude) && (sw.longitude <= ne.longitude);
    }

    double south() const noexcept
    {
        return sw.latitude;
    }

    double west() const noexcept
    {
        return sw.longitude;
    }

    double north() const noexcept
    {
        return ne.latitude;
    }

    double east() const noexcept
    {
        return ne.longitude;
    }

    Coordinate southwest() const noexcept
    {
        return sw;
    }

    Coordinate northeast() const noexcept
    {
        return ne;
    }

    Coordinate southeast() const noexcept
    {
        return {south(), east()};
    }

    Coordinate northwest() const noexcept
    {
        return {north(), west()};
    }

    Coordinate center() const noexcept
    {
        return {(sw.latitude + ne.latitude) / 2, (sw.longitude + ne.longitude) / 2};
    }

    Coordinate constrain(const Coordinate& p) const;

    void extend(const Coordinate& point)
    {
        sw = Coordinate(std::min(point.latitude, sw.latitude),
                        std::min(point.longitude, sw.longitude));
        ne = Coordinate(std::max(point.latitude, ne.latitude),
                        std::max(point.longitude, ne.longitude));
    }

    void extend(const LatLonBounds& bounds)
    {
        extend(bounds.sw);
        extend(bounds.ne);
    }

    bool isEmpty() const
    {
        return sw.latitude > ne.latitude ||
               sw.longitude > ne.longitude;
    }

    bool crosses_antimeridian() const
    {
        return (sw.wrapped().longitude > ne.wrapped().longitude);
    }

    bool contains(const Coordinate& point) const;
    bool contains(const LatLonBounds& area) const;

    bool intersects(LatLonBounds area) const;

private:
    Coordinate sw;
    Coordinate ne;
    bool bounded = true;

    LatLonBounds(Coordinate sw_, Coordinate ne_)
    : sw(sw_), ne(ne_)
    {
    }

    bool contains_latitude(double latitude) const;
    bool contains_longitude(double longitude) const;

    friend bool operator==(const LatLonBounds& a, const LatLonBounds& b)
    {
        return (!a.bounded && !b.bounded) || (a.bounded && b.bounded && a.sw == b.sw && a.ne == b.ne);
    }

    friend bool operator!=(const LatLonBounds& a, const LatLonBounds& b)
    {
        return !(a == b);
    }
};

}
