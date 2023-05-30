#include "infra/tile.h"

#ifdef HAVE_GDAL
#include "infra/gdalalgo.h"
#endif

namespace inf {

GeoMetadata create_xyz_tile_aligned_extent(const inf::GeoMetadata& extent)
{
#ifdef HAVE_GDAL
    static constexpr const int32_t zoomLevel = 10;

    if (extent.projection.empty()) {
        throw RuntimeError("No projection information present in extent");
    }

    auto wgs84meta = gdal::warp_metadata(extent, crs::epsg::WGS84WebMercator);

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
    throw RuntimeError("create_xyz_tile_aligned_extent: infra not compiled with gdal support");
#endif
}

}
