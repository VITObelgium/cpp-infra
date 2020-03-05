#include "infra/geometadata.h"
#include "infra/coordinate.h"
#include "infra/crs.h"
#include "infra/gdalio.h"
#include "infra/test/printsupport.h"

#include <doctest/doctest.h>
#include <fstream>

namespace inf::test {

using namespace doctest;
using namespace std::string_literals;

TEST_CASE("GeoMetadata.projectioninfoProjected")
{
    {
        auto meta = gdal::io::read_metadata(fs::u8path(TEST_DATA_DIR) / "epsg31370.tif");
        REQUIRE_FALSE(meta.projection.empty());
        REQUIRE(meta.projected_epsg().has_value());
        CHECK(meta.projected_epsg() == crs::epsg::BelgianLambert72);
        CHECK(meta.geographic_epsg().value() == crs::epsg::Belge72Geo);
        CHECK(meta.projection_frienly_name() == "EPSG:31370");
    }

    {
        auto meta = gdal::io::read_metadata(fs::u8path(TEST_DATA_DIR) / "epsg3857.tif");
        REQUIRE_FALSE(meta.projection.empty());
        REQUIRE(meta.projected_epsg().has_value());
        CHECK(meta.projected_epsg() == crs::epsg::WGS84WebMercator);
        CHECK(meta.geographic_epsg() == crs::epsg::WGS84);
        CHECK(meta.projection_frienly_name() == "EPSG:3857");
    }
}

}
