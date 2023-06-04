#include "infra/tile.h"

#ifdef INFRA_GDAL_ENABLED
#include "infra/gdalalgo.h"
#endif

namespace inf {

static double pixel_size_at_zoom_level(uint32_t zoomLevel) noexcept
{
    const auto tilesPerRow   = std::pow(2, zoomLevel);
    const auto metersPerTile = constants::EARTH_CIRCUMFERENCE_M / tilesPerRow;

    return metersPerTile / TILE_SIZE;
}

static uint32_t zoom_level_for_pixel_size(double pixelSize, bool preferHigher) noexcept
{
    uint32_t zoomLevel = 20;
    while (zoomLevel > 0) {
        auto zoomLevelpixelSize = pixel_size_at_zoom_level(zoomLevel);
        if (pixelSize <= zoomLevelpixelSize) {
            if (pixelSize != zoomLevelpixelSize && preferHigher) {
                // Prefer the higher zoom level
                ++zoomLevel;
            }
            break;
        }

        --zoomLevel;
    }

    return zoomLevel;
}

GeoMetadata create_xyz_tile_aligned_extent(const inf::GeoMetadata& extent)
{
#ifdef INFRA_GDAL_ENABLED

    if (extent.projection.empty()) {
        throw RuntimeError("No projection information present in extent");
    }

    bool preferHigher = extent.rows * extent.cols < (5000 * 5000);

    auto wgs84meta     = gdal::warp_metadata(extent, crs::epsg::WGS84WebMercator);
    uint32_t zoomLevel = zoom_level_for_pixel_size(extent.cell_size_x(), preferHigher);

    auto topLeft     = crs::web_mercator_to_lat_lon(wgs84meta.top_left());
    auto bottomRight = crs::web_mercator_to_lat_lon(wgs84meta.bottom_right());

    auto topLeftTile     = Tile::for_coordinate(topLeft, zoomLevel).web_mercator_bounds();
    auto bottomRightTile = Tile::for_coordinate(bottomRight, zoomLevel).web_mercator_bounds();

    double cellSize = (topLeftTile.bottomRight.x - topLeftTile.topLeft.x) / TILE_SIZE;

    inf::GeoMetadata result;
    result.xll  = topLeftTile.topLeft.x;
    result.yll  = bottomRightTile.bottomRight.y;
    result.rows = truncate<int32_t>(std::ceil((topLeftTile.topLeft.y - bottomRightTile.bottomRight.y) / cellSize));
    result.cols = truncate<int32_t>(std::ceil((bottomRightTile.bottomRight.x - topLeftTile.topLeft.x) / cellSize));
    result.set_cell_size(cellSize);
    result.set_projection_from_epsg(crs::epsg::WGS84WebMercator);
    result.nodata = extent.nodata;
    return result;
#else
    (void)extent;
    throw RuntimeError("create_xyz_tile_aligned_extent: infra not compiled with gdal support");
#endif
}

}
