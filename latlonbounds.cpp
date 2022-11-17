#include "infra/latlonbounds.h"

#include <algorithm>
#include <cassert>
#include <cstdint>

namespace inf {

bool LatLonBounds::contains(const LatLonBounds& area) const
{
    bool containsAreaLatitude = area.north() <= north() && area.south() >= south();
    if (!containsAreaLatitude) {
        return false;
    }

    bool containsUnwrapped = area.east() <= east() && area.west() >= west();
    if (containsUnwrapped) {
        return true;
    }

    return false;
}

bool LatLonBounds::intersects(const LatLonBounds area) const
{
    bool latitudeIntersects = area.north() > south() && area.south() < north();
    if (!latitudeIntersects) {
        return false;
    }

    bool longitudeIntersects = area.east() > west() && area.west() < east();
    if (longitudeIntersects) {
        return true;
    }

    return false;
}

Coordinate LatLonBounds::constrain(const Coordinate& p) const
{
    if (!bounded) {
        return p;
    }

    double lat = p.latitude;
    double lng = p.longitude;

    if (!contains_latitude(lat)) {
        lat = std::clamp(lat, south(), north());
    }

    if (!contains_longitude(lng)) {
        lng = std::clamp(lng, west(), east());
    }

    return Coordinate{lat, lng};
}

bool LatLonBounds::contains_latitude(double latitude) const
{
    return latitude >= south() && latitude <= north();
}

bool LatLonBounds::contains_longitude(double longitude) const
{
    if (longitude >= west() && longitude <= east()) {
        return true;
    }

    return false;
}

}
