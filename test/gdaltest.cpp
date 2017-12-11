#include "infra/gdal.h"

#include <fstream>
#include <gtest/gtest.h>

namespace infra {

TEST(GdalTest, iteratePoints)
{
    auto ds = gdal::DataSet::create(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);

    EXPECT_EQ(1, ds.layerCount());

    int count = 0;
    int index = 1;
    for (const auto& feature : ds.getLayer(0)) {
        EXPECT_EQ(Point<double>(index, index + 1), std::get<Point<double>>(feature.geometry()));

        index += 2;
        ++count;
    }

    EXPECT_EQ(9, count);
}

TEST(GdalTest, fieldInfo)
{
    auto ds    = gdal::DataSet::create(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);
    auto layer = ds.getLayer(0);
    EXPECT_EQ(9, layer.featureCount());
    EXPECT_EQ(1, layer[0].fieldCount());
    EXPECT_EQ("FID", layer[0].fieldDefinition(0).name());
    EXPECT_EQ(typeid(int64_t), layer[0].fieldDefinition(0).type());
}

TEST(GdalTest, getField)
{
    auto ds = gdal::DataSet::create(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);

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
}
