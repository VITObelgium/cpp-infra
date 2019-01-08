#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "gdx/maskedraster.h"
#include "gdx/rasteriterator.h"
#include "gdx/rasterspan.h"

#include <algorithm>

namespace gdx::test {

using namespace testing;

TEST(ValueSpanTest, assignVector)
{
    std::vector<int> data = {1, 2, 3, 4};
    RasterMetadata meta(2, 2, 2);

    auto dataSpan = make_raster_span(data, meta);

    EXPECT_EQ(4, dataSpan.size());
    EXPECT_EQ(2, dataSpan.nodata());

    EXPECT_EQ(1, dataSpan[0]);
    EXPECT_EQ(2, dataSpan[1]);
    EXPECT_EQ(3, dataSpan[2]);
    EXPECT_EQ(4, dataSpan[3]);
}

TEST(ValueSpanTest, assignMaskedRaster)
{
    MaskedRaster<int> raster(RasterMetadata(2, 3, 1), std::vector<int>{1, 2, 3, 4, 5, 6});

    auto dataSpan = make_raster_span(raster, raster.metadata());

    EXPECT_EQ(6, dataSpan.size());
    EXPECT_EQ(1, dataSpan.nodata());

    EXPECT_EQ(1, dataSpan[0]);
    EXPECT_EQ(2, dataSpan[1]);
    EXPECT_EQ(3, dataSpan[2]);
    EXPECT_EQ(4, dataSpan[3]);
    EXPECT_EQ(5, dataSpan[4]);
    EXPECT_EQ(6, dataSpan[5]);

    EXPECT_EQ(1, dataSpan(0, 0));
    EXPECT_EQ(2, dataSpan(0, 1));
    EXPECT_EQ(3, dataSpan(0, 2));
    EXPECT_EQ(4, dataSpan(1, 0));
    EXPECT_EQ(5, dataSpan(1, 1));
    EXPECT_EQ(6, dataSpan(1, 2));
}

TEST(ValueSpanTest, iterateValues)
{
    std::vector<int> data      = {1, 9, 3, 9};
    std::vector<int> valueData = {1, 3};
    RasterMetadata meta(2, 2, 9);

    auto dataSpan = make_raster_span(data, meta);

    EXPECT_EQ(4, std::distance(begin(dataSpan), end(dataSpan)));
    EXPECT_EQ(2, std::distance(value_begin(dataSpan), value_end(dataSpan)));

    EXPECT_THAT(dataSpan, Pointwise(Eq(), data));
}

TEST(ValueSpanTest, is_nodata)
{
    std::vector<int> data = {1, 9, 3, 9};
    RasterMetadata meta(2, 2, 9);

    auto dataSpan = make_raster_span(data, meta);
    EXPECT_FALSE(dataSpan.is_nodata(0));
    EXPECT_TRUE(dataSpan.is_nodata(1));
    EXPECT_FALSE(dataSpan.is_nodata(2));
    EXPECT_TRUE(dataSpan.is_nodata(3));

    EXPECT_FALSE(dataSpan.is_nodata(0, 0));
    EXPECT_TRUE(dataSpan.is_nodata(0, 1));
    EXPECT_FALSE(dataSpan.is_nodata(1, 0));
    EXPECT_TRUE(dataSpan.is_nodata(1, 1));
}
}
