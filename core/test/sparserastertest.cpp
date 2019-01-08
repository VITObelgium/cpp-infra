#include "gdx/test/testbase.h"

#include "gdx/raster.h"

#include <type_traits>

namespace gdx::test {

class SparseRasterTest : public Test
{
protected:
    SparseRasterTest()
    : raster(RasterMetadata(3, 2, -99), std::vector<int>{-99, 0, 1, -99, 5, -99})
    {
    }

    SparseRaster<int> raster;
};

TEST_F(SparseRasterTest, nodataChecks)
{
    EXPECT_TRUE(raster.is_nodata(0, 0));
    EXPECT_FALSE(raster.is_nodata(0, 1));
    EXPECT_FALSE(raster.is_nodata(1, 0));
    EXPECT_TRUE(raster.is_nodata(1, 1));
    EXPECT_FALSE(raster.is_nodata(2, 0));
    EXPECT_TRUE(raster.is_nodata(2, 1));

    EXPECT_TRUE(raster.is_nodata(0));
    EXPECT_FALSE(raster.is_nodata(1));
    EXPECT_FALSE(raster.is_nodata(2));
    EXPECT_TRUE(raster.is_nodata(3));
    EXPECT_FALSE(raster.is_nodata(4));
    EXPECT_TRUE(raster.is_nodata(5));
}

TEST_F(SparseRasterTest, elementAccess)
{
    EXPECT_EQ(0, raster(0, 1));
    EXPECT_EQ(1, raster(1, 0));
    EXPECT_EQ(5, raster(2, 0));

    raster(2, 0) = 6;
    EXPECT_EQ(6, raster(2, 0));
}

TEST_F(SparseRasterTest, compare)
{
    SparseRaster<int> copy = raster.copy();
    EXPECT_EQ(raster, copy);

    copy(2, 0) = 6;
    EXPECT_NE(raster, copy);
}

TEST_F(SparseRasterTest, multiply)
{
    SparseRaster<int> expected(RasterMetadata(3, 2, -99), std::vector<int>{-99, 0, 2, -99, 10, -99});
    EXPECT_EQ(expected, raster * 2);

    raster *= 2;
    EXPECT_EQ(expected, raster);
}

TEST_F(SparseRasterTest, add)
{
    SparseRaster<int> expected(RasterMetadata(3, 2, -99), std::vector<int>{-99, 1, 2, -99, 6, -99});
    EXPECT_EQ(expected, raster + 1);

    raster += 1;
    EXPECT_EQ(expected, raster);
}

TEST_F(SparseRasterTest, subtract)
{
    SparseRaster<int> expected(RasterMetadata(3, 2, -99), std::vector<int>{-99, -1, 0, -99, 4, -99});
    EXPECT_EQ(expected, raster - 1);

    raster -= 1;
    EXPECT_EQ(expected, raster);
}

TEST_F(SparseRasterTest, negate)
{
    SparseRaster<int> expected(RasterMetadata(3, 2, -99), std::vector<int>{-99, 0, -1, -99, -5, -99});
    EXPECT_EQ(expected, -raster);
}

}
