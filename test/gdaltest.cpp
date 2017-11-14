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
    EXPECT_EQ(typeid(long long), layer[0].fieldDefinition(0).type());
}

TEST(GdalTest, getField)
{
    auto ds = gdal::DataSet::create(TEST_DATA_DIR "/points.shp", gdal::VectorType::ShapeFile);

    int index = 0;
    for (const auto& feature : ds.getLayer(0)) {
        EXPECT_EQ(index, feature.getFieldAs<int>(1));
        EXPECT_EQ(index, feature.getFieldAs<long long>(1));
        EXPECT_DOUBLE_EQ(double(index), feature.getFieldAs<double>(1));
        EXPECT_FLOAT_EQ(float(index), feature.getFieldAs<float>(1));
        EXPECT_EQ(std::to_string(index), feature.getFieldAs<std::string_view>(1));

        EXPECT_EQ(index, feature.getFieldAs<int>("FID"));
        EXPECT_EQ(index, feature.getFieldAs<long long>("FID"));
        EXPECT_DOUBLE_EQ(double(index), feature.getFieldAs<double>("FID"));
        EXPECT_FLOAT_EQ(float(index), feature.getFieldAs<float>("FID"));
        EXPECT_EQ(std::to_string(index), feature.getFieldAs<std::string_view>("FID"));

        ++index;
    }
}
}
