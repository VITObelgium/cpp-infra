#include "infra/gdal.h"
#include "infra/crs.h"
#include "infra/gdalio.h"

#include <doctest/doctest.h>
#include <fstream>

namespace inf::test {

using namespace doctest;
using namespace std::string_literals;

TEST_CASE("Gdal.iteratePoints")
{
    auto ds = gdal::VectorDataSet::open(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);

    CHECK(ds.layer_count() == 1);

    int count = 0;
    int index = 1;
    for (const auto& feature : ds.layer(0)) {
        auto geometry = feature.geometry();
        CHECK(gdal::Geometry::Type::Point == geometry.type());

        auto point = geometry.as<gdal::PointGeometry>();
        CHECK(Point<double>(index, index + 1) == point.point());

        index += 2;
        ++count;
    }

    CHECK(count == 9);
}

TEST_CASE("Gdal.fieldInfo")
{
    auto ds    = gdal::VectorDataSet::open(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);
    auto layer = ds.layer(0);
    CHECK(layer.feature_count() == 9);
    CHECK(layer.feature(0).field_count() == 1);
    CHECK("FID"s == layer.feature(0).field_definition(0).name());
    CHECK(typeid(int64_t) == layer.feature(0).field_definition(0).type());
}

TEST_CASE("Gdal.getField")
{
    auto ds = gdal::VectorDataSet::open(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);

    int index = 0;
    for (const auto& feature : ds.layer(0)) {
        CHECK(index == feature.field_as<int32_t>(1));
        CHECK(index == feature.field_as<int64_t>(1));
        CHECK(double(index) == Approx(feature.field_as<double>(1)));
        CHECK(float(index) == Approx(feature.field_as<float>(1)));
        CHECK(std::to_string(index) == feature.field_as<std::string_view>(1));

        CHECK(index == feature.field_as<int32_t>("FID"));
        CHECK(index == feature.field_as<int64_t>("FID"));
        CHECK(double(index) == Approx(feature.field_as<double>("FID")));
        CHECK(float(index) == Approx(feature.field_as<float>("FID")));
        CHECK(std::to_string(index) == feature.field_as<std::string_view>("FID"));

        ++index;
    }
}

TEST_CASE("Gdal.inMemoryVector")
{
    auto memDriver = gdal::VectorDriver::create(gdal::VectorType::Memory);
    gdal::VectorDataSet ds(memDriver.create_dataset());
    auto layer = ds.create_layer("wbv", gdal::Geometry::Type::Unknown);

    auto nameField  = gdal::FieldDefinition::create<std::string>("name");
    auto valueField = gdal::FieldDefinition::create<double>("value");
    layer.create_field(nameField);
    layer.create_field(valueField);

    auto nameIndex  = layer.layer_definition().field_index(nameField.name());
    auto valueIndex = layer.layer_definition().field_index(valueField.name());

    {
        gdal::Feature feat(layer.layer_definition());
        feat.set_field<std::string>(nameIndex, "name1");
        feat.set_field<double>(valueIndex, 4.0);
        layer.create_feature(feat);
    }

    {
        gdal::Feature feat(layer.layer_definition());
        feat.set_field<std::string>(nameIndex, "name2");
        layer.create_feature(feat);
    }

    for (auto& feature : ds.layer(0)) {
        if (feature.field_as<std::string_view>(nameIndex) == "name1") {
            CHECK(feature.field_as<double>(valueIndex) == 4.0);
            CHECK(feature.opt_field_as<double>(valueIndex) == 4.0);
        } else if (feature.field_as<std::string_view>(nameIndex) == "name2") {
            CHECK_FALSE(feature.field_is_valid(valueIndex));
            CHECK_FALSE(feature.opt_field_as<double>(valueIndex).has_value());
        } else {
            FAIL("Unexpected field in layer");
        }
    }
}

TEST_CASE("Gdal.convertPointProjected")
{
    // Check conversion of bottom left corner of flanders map

    // 22000.000 153000.000 (x,y) lambert 72 EPSG:31370 should be converted to 2.55772472781224 50.6735631138308 (lat,long) EPSG:4326

    // Bottom left coordinate
    auto point = gdal::convert_point_projected(crs::epsg::BelgianLambert72, crs::epsg::WGS84, Point<double>(22000.000, 153000.000));
    CHECK(2.55772472781224 == Approx(point.x).epsilon(1e-6));
    CHECK(50.6735631138308 == Approx(point.y).epsilon(1e-6));
}

TEST_CASE("Gdal.createExcelFile")
{
    if (!gdal::VectorDriver::is_supported(gdal::VectorType::Xlsx)) {
        return;
    }

    auto shapeDriver = gdal::VectorDriver::create(gdal::VectorType::Xlsx);
    auto ds          = shapeDriver.create_dataset("sheet.xlsx");

    // Each field definition defines a column in the spreadsheet
    auto layer   = ds.create_layer("Workbook");
    auto col1Def = gdal::FieldDefinition("Column1", typeid(std::string));
    auto col2Def = gdal::FieldDefinition("Column2", typeid(int32_t));

    layer.create_field(col1Def);
    layer.create_field(col2Def);

    // Each feature is a line in the spreadsheet

    // Values can be set using column numbers
    gdal::Feature feat1(layer.layer_definition());
    feat1.set_field(0, "Test line 1");
    feat1.set_field(1, 1);
    layer.create_feature(feat1);

    // Values can be set using column names
    gdal::Feature feat2(layer.layer_definition());
    feat2.set_field("Column1", "Test line 2");
    feat2.set_field("Column2", 2);
    layer.create_feature(feat2);
}

TEST_CASE("Gdal.utf8Path")
{
    if (!gdal::RasterDriver::is_supported(gdal::RasterType::Netcdf)) {
        return;
    }

    CHECK_NOTHROW(gdal::RasterDataSet::open(fs::u8path("NETCDF:\"" TEST_DATA_DIR "/België/latlon.nc\":lat"), gdal::RasterType::Netcdf));

    auto ds          = gdal::RasterDataSet::open(fs::u8path(TEST_DATA_DIR "/België/latlon.nc"), gdal::RasterType::Netcdf);
    auto subDatasets = ds.metadata("SUBDATASETS");
    CHECK(subDatasets.size() == 4u);
}

TEST_CASE("Gdal.spatialReference")
{
    {
        auto meta = gdal::io::read_metadata(fs::u8path(TEST_DATA_DIR) / "epsg31370.tif");
        gdal::SpatialReference srs(meta.projection);
        CHECK(srs.is_projected());
        CHECK_FALSE(srs.is_geographic());
        CHECK(srs.epsg_cs() == crs::epsg::BelgianLambert72);
        CHECK(srs.epsg_geog_cs() == crs::epsg::Belge72Geo);
    }

    {
        auto meta = gdal::io::read_metadata(fs::u8path(TEST_DATA_DIR) / "epsg3857.tif");
        gdal::SpatialReference srs(meta.projection);
        CHECK(srs.is_projected());
        CHECK_FALSE(srs.is_geographic());
        CHECK(srs.epsg_cs() == crs::epsg::WGS84WebMercator);
        CHECK(srs.epsg_geog_cs() == crs::epsg::WGS84);
    }
}

}
