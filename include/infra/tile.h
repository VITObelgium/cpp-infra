#pragma once

#include "infra/algo.h"
#include "infra/cast.h"
#include "infra/coordinate.h"
#include "infra/crs.h"
#include "infra/geoconstants.h"
#include "infra/geometadata.h"
#include "infra/latlonbounds.h"
#include "infra/math.h"
#include "infra/rect.h"

#include <cassert>
#include <cstdint>

namespace inf {

constexpr const uint16_t TILE_SIZE = 256;

// An XYZ web mercator tile
class Tile
{
public:
    static inf::Point<double> _xy(inf::Coordinate coord)
    {
        inf::Point<double> result;
        result.x = coord.longitude / 360.0 + 0.5;

        auto sinlat = std::sin(inf::math::deg_to_rad(coord.latitude));
        result.y    = 0.5 - 0.25 * std::log((1.0 + sinlat) / (1.0 - sinlat)) / inf::math::pi;

        return result;
    }

    static Tile for_coordinate(inf::Coordinate coord, int32_t zoom)
    {
        uint32_t tilex = 0;
        uint32_t tiley = 0;

        // const auto [x, y] = lat_lon_to_web_mercator(coord);
        const auto [x, y] = _xy(coord);
        const auto z2     = std::pow(2, zoom);

        if (x <= 0) {
            tilex = 0;
        } else if (x >= 1)
            tilex = int32_t(z2 - 1);
        else {
            // To address loss of precision in round-tripping between tile
            // and lng/lat, points within EPSILON of the right side of a tile
            // are counted in the next tile over.
            tilex = int32_t(std::floor((x + std::numeric_limits<double>::epsilon()) * z2));
        }

        if (y <= 0) {
            tiley = 0;
        } else if (y >= 1) {
            tiley = int32_t(z2 - 1);
        } else {
            tiley = int32_t(std::floor((y + std::numeric_limits<double>::epsilon()) * z2));
        }

        return Tile(tilex, tiley, zoom);
    }

    Tile() noexcept = default;
    Tile(int32_t x, int32_t y, int32_t z)
    : _x(x), _y(y), _z(z)
    {
    }

    int32_t x() const noexcept
    {
        return _x;
    }

    int32_t y() const noexcept
    {
        return _y;
    }

    int32_t z() const noexcept
    {
        return _z;
    }

    inf::Coordinate upper_left() const
    {
        const auto z2         = std::pow(2, _z);
        const auto lonDegrees = _x / z2 * 360.0 - 180.0;
        const auto latRad     = std::atan(std::sinh(inf::math::pi * (1 - 2 * _y / z2)));

        return inf::Coordinate(inf::math::rad_to_deg(latRad), lonDegrees);
    }

    inf::Coordinate center() const
    {
        const auto z2             = std::pow(2, _z);
        const auto degreesPerTile = 360 / z2;

        const auto lonDegrees = _x / z2 * 360.0 - 180.0;
        const auto latRad     = std::atan(std::sinh(inf::math::pi * (1 - 2 * _y / z2)));

        return inf::Coordinate(inf::math::rad_to_deg(latRad) - degreesPerTile / 2, lonDegrees + degreesPerTile / 2);
    }

    // bounds in projected meters EPSG:3857
    inf::Rect<double> web_mercator_bounds() const noexcept
    {
        inf::Rect<double> result;

        const auto tileSize = constants::EARTH_CIRCUMFERENCE_M / std::pow(2, _z);
        const auto left     = (_x * tileSize) - (constants::EARTH_CIRCUMFERENCE_M / 2.0);
        const auto right    = left + tileSize;

        const auto top    = (constants::EARTH_CIRCUMFERENCE_M / 2.0) - (_y * tileSize);
        const auto bottom = top - tileSize;

        result.topLeft     = inf::Point(left, top);
        result.bottomRight = inf::Point(right, bottom);

        return result;
    }

    // bounds in degrees EPSG:4326
    LatLonBounds bounds() const
    {
        const auto z2 = std::pow(2, _z);

        const auto ulLonDeg = _x / z2 * 360.0 - 180.0;
        const auto ulLatRad = std::atan(std::sinh(inf::math::pi * (1 - 2 * _y / z2)));

        const auto lrLonDeg = (_x + 1) / z2 * 360.0 - 180.0;
        const auto lrLatRad = std::atan(std::sinh(inf::math::pi * (1 - 2 * (_y + 1) / z2)));

        return LatLonBounds::hull(inf::Coordinate(inf::math::rad_to_deg(ulLatRad), ulLonDeg),
                                  inf::Coordinate(inf::math::rad_to_deg(lrLatRad), lrLonDeg));
    }

    std::array<Tile, 4> direct_children() const
    {
        std::array<Tile, 4> result;
        result[0] = Tile(_x * 2, _y * 2, _z + 1);
        result[1] = Tile(_x * 2 + 1, _y * 2, _z + 1);
        result[2] = Tile(_x * 2 + 1, _y * 2 + 1, _z + 1);
        result[3] = Tile(_x * 2, _y * 2 + 1, _z + 1);
        return result;
    }

    std::vector<Tile> children(int32_t targetZoom) const
    {
        std::vector<Tile> result;

        if (_z < targetZoom) {
            for (auto& child : direct_children()) {
                result.push_back(child);
                inf::append_to_container(result, child.children(targetZoom));
            }
        }

        return result;
    }

    // Invokes the callback for the current tile and all it's child tiles
    // untill the zoom level is reached
    // traversal of the current tile ends when the callback returns false

    template <typename Callable>
    inline void traverse(int32_t targetZoom, Callable&& cb)
    {
        if (_z >= targetZoom) {
            return;
        }

        if (!cb(*this)) {
            return;
        }

        for (auto& child : direct_children()) {
            if (cb(child)) {
                child.traverse(targetZoom, cb);
            }
        }
    }

private:
    int32_t _x = 0;
    int32_t _y = 0;
    int32_t _z = 0;
};

// returns the 0 based index of the tile in the given extent
// tiles are row major, 0 is the top left tile
inline std::optional<uint32_t> tile_index(const Tile& tile, const inf::GeoMetadata& meta) noexcept
{
    std::optional<uint32_t> result;

    auto upperLeft = crs::lat_lon_to_web_mercator(tile.center());

    if (meta.is_on_map(upperLeft)) {
        const auto tilesPerRow = inf::truncate<uint32_t>(meta.cols / TILE_SIZE);

        auto cell = meta.convert_point_to_cell(upperLeft);
        cell.r /= TILE_SIZE;
        cell.c /= TILE_SIZE;
        result = cell.r * tilesPerRow + cell.c;
    }

    return result;
}

GeoMetadata create_xyz_tile_aligned_extent(const inf::GeoMetadata& extent);

}
