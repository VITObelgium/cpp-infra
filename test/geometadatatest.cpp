#include "infra/geometadata.h"
#include "infra/crs.h"
#include "infra/gdalio.h"
#include "infra/test/printsupport.h"

#include <doctest/doctest.h>
#include <fstream>

namespace inf::test {

using namespace doctest;
using namespace std::string_literals;

TEST_CASE("GeoMetadata")
{
    SUBCASE("projectioninfoProjected 31370")
    {
        auto meta = gdal::io::read_metadata(fs::u8path(TEST_DATA_DIR) / "epsg31370.tif");
        REQUIRE_FALSE(meta.projection.empty());
        REQUIRE(meta.projected_epsg().has_value());
        CHECK(meta.projected_epsg() == crs::epsg::BelgianLambert72);
        CHECK(meta.geographic_epsg().value() == crs::epsg::Belge72Geo);
        CHECK(meta.projection_frienly_name() == "EPSG:31370");
    }

    SUBCASE("projectioninfoProjected 3857")
    {
        auto meta = gdal::io::read_metadata(fs::u8path(TEST_DATA_DIR) / "epsg3857.tif");
        REQUIRE_FALSE(meta.projection.empty());
        REQUIRE(meta.projected_epsg().has_value());
        CHECK(meta.projected_epsg() == crs::epsg::WGS84WebMercator);
        CHECK(meta.geographic_epsg() == crs::epsg::WGS84);
        CHECK(meta.projection_frienly_name() == "EPSG:3857");
    }

    SUBCASE("bounding box zero origin")
    {
        GeoMetadata meta(10, 5, {});
        meta.xll = 0;
        meta.yll = 0;
        meta.set_cell_size(5.0);

        auto bbox = meta.bounding_box();
        CHECK(bbox.topLeft == Point(0.0, 50.0));
        CHECK(bbox.bottomRight == Point(25.0, 0.0));
    }

    SUBCASE("bounding box negative y origin")
    {
        GeoMetadata meta(2, 2);
        meta.xll = 9.0;
        meta.yll = -10.0;
        meta.set_cell_size(4.0);

        auto bbox = meta.bounding_box();
        CHECK(bbox.topLeft == Point(9.0, -2.0));
        CHECK(bbox.bottomRight == Point(17.0, -10.0));
    }

    SUBCASE("point calculations zero origin")
    {
        GeoMetadata meta(2, 2);
        meta.xll = 0.0;
        meta.yll = 0.0;
        meta.set_cell_size(1.0);

        CHECK(Point(0.5, 1.5) == meta.convert_cell_centre_to_xy(Cell(0, 0)));
        CHECK(Point(1.5, 0.5) == meta.convert_cell_centre_to_xy(Cell(1, 1)));

        CHECK(meta.convert_col_ll_to_x(0) == 0.0);
        CHECK(meta.convert_row_ll_to_y(0) == 1.0);
        CHECK(meta.convert_col_ll_to_x(2) == 2.0);
        CHECK(meta.convert_row_ll_to_y(2) == -1.0);

        CHECK(meta.convert_x_to_col_fraction(0) == 0.0);
        CHECK(meta.convert_y_to_row_fraction(0) == 2.0);
        CHECK(meta.convert_x_to_col_fraction(2) == 2.0);
        CHECK(meta.convert_y_to_row_fraction(2) == 0.0);

        CHECK(Point(0.0, 2.0) == meta.top_left());
        CHECK(Point(1.0, 1.0) == meta.center());
        CHECK(Point(2.0, 0.0) == meta.bottom_right());
    }

    SUBCASE("point calculations non negative origin")
    {
        GeoMetadata meta(2, 2);
        meta.xll = -1.0;
        meta.yll = -1.0;
        meta.set_cell_size(1.0);

        CHECK(Point(-0.5, 0.5) == meta.convert_cell_centre_to_xy(Cell(0, 0)));
        CHECK(Point(0.5, -0.5) == meta.convert_cell_centre_to_xy(Cell(1, 1)));

        CHECK(meta.convert_col_ll_to_x(0) == -1.0);
        CHECK(meta.convert_row_ll_to_y(0) == 0.0);
        CHECK(meta.convert_col_ll_to_x(2) == 1.0);
        CHECK(meta.convert_row_ll_to_y(2) == -2.0);

        CHECK(meta.convert_x_to_col_fraction(0) == 1.0);
        CHECK(meta.convert_y_to_row_fraction(0) == 1.0);
        CHECK(meta.convert_x_to_col_fraction(2) == 3.0);
        CHECK(meta.convert_y_to_row_fraction(2) == -1.0);

        CHECK(Point(-1.0, 1.0) == meta.top_left());
        CHECK(Point(0.0, 0.0) == meta.center());
        CHECK(Point(1.0, -1.0) == meta.bottom_right());
    }

    SUBCASE("point calculations non positive origin")
    {
        GeoMetadata meta(2, 2);
        meta.xll = 1.0;
        meta.yll = 1.0;
        meta.set_cell_size(1.0);

        CHECK(Point(1.5, 2.5) == meta.convert_cell_centre_to_xy(Cell(0, 0)));
        CHECK(Point(2.5, 1.5) == meta.convert_cell_centre_to_xy(Cell(1, 1)));

        CHECK(Point(1.0, 3.0) == meta.top_left());
        CHECK(Point(2.0, 2.0) == meta.center());
        CHECK(Point(3.0, 1.0) == meta.bottom_right());
    }
}

}
