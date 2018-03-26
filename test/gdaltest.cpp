#include "infra/gdal.h"

#include <fstream>
#include <gtest/gtest.h>

namespace infra {

TEST(GdalTest, iteratePoints)
{
    auto ds = gdal::VectorDataSet::create(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);

    EXPECT_EQ(1, ds.layerCount());

    int count = 0;
    int index = 1;
    for (const auto& feature : ds.getLayer(0)) {
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
    auto layer = ds.getLayer(0);
    EXPECT_EQ(9, layer.featureCount());
    EXPECT_EQ(1, layer.feature(0).fieldCount());
    EXPECT_EQ("FID", layer.feature(0).fieldDefinition(0).name());
    EXPECT_EQ(typeid(int64_t), layer.feature(0).fieldDefinition(0).type());
}

TEST(GdalTest, getField)
{
    auto ds = gdal::VectorDataSet::create(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);

    int index = 0;
    for (const auto& feature : ds.getLayer(0)) {
        EXPECT_EQ(index, feature.getFieldAs<int32_t>(1));
        EXPECT_EQ(index, feature.getFieldAs<int64_t>(1));
        EXPECT_DOUBLE_EQ(double(index), feature.getFieldAs<double>(1));
        EXPECT_FLOAT_EQ(float(index), feature.getFieldAs<float>(1));
        EXPECT_EQ(std::to_string(index), feature.getFieldAs<std::string_view>(1));

        EXPECT_EQ(index, feature.getFieldAs<int32_t>("FID"));
        EXPECT_EQ(index, feature.getFieldAs<int64_t>("FID"));
        EXPECT_DOUBLE_EQ(double(index), feature.getFieldAs<double>("FID"));
        EXPECT_FLOAT_EQ(float(index), feature.getFieldAs<float>("FID"));
        EXPECT_EQ(std::to_string(index), feature.getFieldAs<std::string_view>("FID"));

        ++index;
    }
}

TEST(GdalTest, convertPointProjected)
{
    // Check conversion of bottom left corner of flanders map

    // 22000.000 153000.000 (x,y) lambert 72 EPSG:31370 should be converted to 2.55772472781224 50.6735631138308 (lat,long) EPSG:4326

    // Bottom left coordinate
    auto point = gdal::convertPointProjected(31370, 4326, Point<double>(22000.000, 153000.000));
    EXPECT_NEAR(2.55772472781224, point.x, 1e-10);
    EXPECT_NEAR(50.6735631138308, point.y, 1e-10);
}

TEST(GdalTest, createExcelFile)
{
    auto shapeDriver = gdal::VectorDriver::create(gdal::VectorType::Xlsx);
    auto ds          = shapeDriver.createDataSet("sheet.xlsx");

    // Each field definition defines a column in the spreadsheet
    auto layer   = ds.createLayer("Workbook");
    auto col1Def = gdal::FieldDefinition("Column1", typeid(std::string));
    auto col2Def = gdal::FieldDefinition("Column2", typeid(int32_t));

    layer.createField(col1Def);
    layer.createField(col2Def);

    // Each feature is a line in the spreadsheet

    // Values can be set using column numbers
    gdal::Feature feat1(layer.layerDefinition());
    feat1.setField(0, "Test line 1");
    feat1.setField(1, 1);
    layer.createFeature(feat1);

    // Values can be set using column names
    gdal::Feature feat2(layer.layerDefinition());
    feat2.setField("Column1", "Test line 2");
    feat2.setField("Column2", 2);
    layer.createFeature(feat2);
}
}
