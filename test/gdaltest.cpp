#include "infra/gdal.h"

#include <fstream>
#include <gtest/gtest.h>

namespace infra {

TEST(GdalTest, iteratePoints)
{
    gdal::Registration reg;

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
}
