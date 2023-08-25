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
        auto meta = gdal::io::read_metadata(file::u8path(TEST_DATA_DIR) / "epsg31370.tif");
        REQUIRE_FALSE(meta.projection.empty());
        REQUIRE(meta.projected_epsg().has_value());
        CHECK(meta.projected_epsg() == crs::epsg::BelgianLambert72);
        CHECK(meta.geographic_epsg().value() == crs::epsg::Belge72Geo);
        CHECK(meta.projection_frienly_name() == "EPSG:31370");
    }

    SUBCASE("projectioninfoProjected 3857")
    {
        auto meta = gdal::io::read_metadata(file::u8path(TEST_DATA_DIR) / "epsg3857.tif");
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

        CHECK(meta.convert_x_to_col_fraction(-1) == -1.0);
        CHECK(meta.convert_x_to_col_fraction(0) == 0.0);
        CHECK(meta.convert_x_to_col_fraction(2) == 2.0);
        CHECK(meta.convert_x_to_col_fraction(3) == 3.0);

        CHECK(meta.convert_y_to_row_fraction(-1) == 3.0);
        CHECK(meta.convert_y_to_row_fraction(0) == 2.0);
        CHECK(meta.convert_y_to_row_fraction(2) == 0.0);
        CHECK(meta.convert_y_to_row_fraction(3) == -1.0);

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

    SUBCASE("intersect meta: epsg 4326")
    {
        const std::array<double, 6> trans{-30.000000763788108, 0.10000000169730690, 0.0, 29.999999619212282, 0.0, 0.049999998635984290};

        GeoMetadata meta(840, 900);
        gdal::fill_geometadata_from_geo_transform(meta, trans);

        CHECK(meta.convert_col_centre_to_x(0) == Approx(trans[0] + (trans[1] / 2)));
        CHECK(meta.convert_row_centre_to_y(0) == Approx(trans[3] + (trans[5] / 2)));

        const auto col0X = meta.convert_col_centre_to_x(0);
        const auto row0Y = meta.convert_row_centre_to_y(0);

        CHECK(meta.convert_x_to_col(col0X) == 0);
        CHECK(meta.convert_y_to_row(row0Y) == 0);

        const auto cutout = gdal::io::detail::intersect_metadata(meta, meta);
        CHECK(cutout.cols == 900);
        CHECK(cutout.rows == 840);

        CHECK(cutout.srcColOffset == 0);
        CHECK(cutout.dstColOffset == 0);

        CHECK(cutout.srcRowOffset == 0);
        CHECK(cutout.dstRowOffset == 0);
    }

    SUBCASE("metadata intersects")
    {
        GeoMetadata meta(3, 3, 0.0, 0.0, 5.0, {});

        CHECK(metadata_intersects(meta, GeoMetadata(3, 3, 10.0, 10.0, 5.0, {})));
        CHECK(metadata_intersects(meta, GeoMetadata(3, 3, -10.0, -10.0, 5.0, {})));
        CHECK(metadata_intersects(meta, GeoMetadata(3, 3, -10.0, 10.0, 5.0, {})));
        CHECK(metadata_intersects(meta, GeoMetadata(3, 3, 10.0, -10.0, 5.0, {})));

        CHECK(!metadata_intersects(meta, GeoMetadata(3, 3, 15.0, 15.0, 5.0, {})));
        CHECK(!metadata_intersects(meta, GeoMetadata(3, 3, 0.0, 15.0, 5.0, {})));
        CHECK(!metadata_intersects(meta, GeoMetadata(3, 3, 15.0, 0.0, 5.0, {})));
        CHECK(!metadata_intersects(meta, GeoMetadata(3, 3, 0.0, -15.0, 5.0, {})));
    }

    SUBCASE("metadata intersects only y overlap")
    {
        GeoMetadata meta1(133, 121, 461144.59164446819, 6609204.0877060490, 76.437028285176211, {});
        GeoMetadata meta2(195, 122, 475361.87890551100, 6607216.7249706341, 76.437028285176211, {});
        CHECK(!metadata_intersects(meta1, meta2));
    }

    SUBCASE("metadata intersects only x overlap")
    {
        GeoMetadata meta1(133, 121, 461144.59164446819, 6609204.0877060490, 76.437028285176211, {});
        GeoMetadata meta2(195, 122, 461144.59164446819, 6807216.7249706341, 76.437028285176211, {});
        CHECK(!metadata_intersects(meta1, meta2));
    }

    SUBCASE("metadata intersects different but alligned cellsize")
    {
        GeoMetadata meta(3, 3, 0.0, 0.0, 10.0, {});

        CHECK(metadata_intersects(meta, GeoMetadata(4, 4, 10.0, 10.0, 5.0, {})));
        CHECK(!metadata_intersects(meta, GeoMetadata(4, 4, 30.0, 30.0, 5.0, {})));

        CHECK_THROWS_AS(metadata_intersects(meta, GeoMetadata(4, 4, 11.0, 10.0, 5.0, {})), InvalidArgument);
        CHECK_THROWS_AS(metadata_intersects(meta, GeoMetadata(4, 4, 10.0, 11.0, 5.0, {})), InvalidArgument);
        CHECK_THROWS_AS(metadata_intersects(GeoMetadata(4, 4, 11.0, 10.0, 5.0, {}), meta), InvalidArgument);
        CHECK_THROWS_AS(metadata_intersects(GeoMetadata(4, 4, 10.0, 11.0, 5.0, {}), meta), InvalidArgument);
    }
}

}
