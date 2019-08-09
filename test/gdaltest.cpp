#include "infra/gdal.h"

#include <fstream>
#include <gtest/gtest.h>

namespace inf::test {

TEST(GdalTest, iteratePoints)
{
    auto ds = gdal::VectorDataSet::create(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);

    EXPECT_EQ(1, ds.layer_count());

    int count = 0;
    int index = 1;
    for (const auto& feature : ds.layer(0)) {
        auto geometry = feature.geometry();
        EXPECT_EQ(gdal::Geometry::Type::Point, geometry.type());

        auto point = geometry.as<gdal::PointGeometry>();
        EXPECT_EQ(Point<double>(index, index + 1), point.point());

        index += 2;
        ++count;
    }

    EXPECT_EQ(9, count);
}

TEST(GdalTest, fieldInfo)
{
    auto ds    = gdal::VectorDataSet::create(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);
    auto layer = ds.layer(0);
    EXPECT_EQ(9, layer.feature_count());
    EXPECT_EQ(1, layer.feature(0).field_count());
    EXPECT_STREQ("FID", layer.feature(0).field_definition(0).name());
    EXPECT_EQ(typeid(int64_t), layer.feature(0).field_definition(0).type());
}

TEST(GdalTest, getField)
{
    auto ds = gdal::VectorDataSet::create(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);

    int index = 0;
    for (const auto& feature : ds.layer(0)) {
        EXPECT_EQ(index, feature.field_as<int32_t>(1));
        EXPECT_EQ(index, feature.field_as<int64_t>(1));
        EXPECT_DOUBLE_EQ(double(index), feature.field_as<double>(1));
        EXPECT_FLOAT_EQ(float(index), feature.field_as<float>(1));
        EXPECT_EQ(std::to_string(index), feature.field_as<std::string_view>(1));

        EXPECT_EQ(index, feature.field_as<int32_t>("FID"));
        EXPECT_EQ(index, feature.field_as<int64_t>("FID"));
        EXPECT_DOUBLE_EQ(double(index), feature.field_as<double>("FID"));
        EXPECT_FLOAT_EQ(float(index), feature.field_as<float>("FID"));
        EXPECT_EQ(std::to_string(index), feature.field_as<std::string_view>("FID"));

        ++index;
    }
}

TEST(GdalTest, convertPointProjected)
{
    // Check conversion of bottom left corner of flanders map

    // 22000.000 153000.000 (x,y) lambert 72 EPSG:31370 should be converted to 2.55772472781224 50.6735631138308 (lat,long) EPSG:4326

    // Bottom left coordinate
    auto point = gdal::convert_point_projected(31370, 4326, Point<double>(22000.000, 153000.000));
    EXPECT_NEAR(2.55772472781224, point.x, 1e-8);
    EXPECT_NEAR(50.6735631138308, point.y, 1e-8);
}

TEST(GdalTest, createExcelFile)
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

TEST(GdalTest, utf8Path)
{
    if (!gdal::RasterDriver::is_supported(gdal::RasterType::Netcdf)) {
        return;
    }

    EXPECT_NO_THROW(gdal::RasterDataSet::create(fs::u8path("NETCDF:\"" TEST_DATA_DIR "/België/latlon.nc\":lat"), gdal::RasterType::Netcdf));

    auto ds          = gdal::RasterDataSet::create(fs::u8path(TEST_DATA_DIR "/België/latlon.nc"), gdal::RasterType::Netcdf);
    auto subDatasets = ds.metadata("SUBDATASETS");
    EXPECT_EQ(4u, subDatasets.size());
}

}
