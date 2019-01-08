#include "gdx/algo/clustersize.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

template <class RasterType>
class ClusterSizeTest : public TestBase<RasterType>
{
};

TYPED_TEST_CASE(ClusterSizeTest, RasterTypes);

TEST(ClusterSizeTest, clusterSize)
{
    const MaskedRaster<int32_t> ras(5, 4, std::vector<int32_t>{1, 1, 1, 1, 1, 1, 2, 3, 3, 3, 3, 3, 1, 1, 5, 5, 1, 1, 5, 1});

    const MaskedRaster<int32_t> expected(ras.metadata(), std::vector<int32_t>{
                                                            6, 6, 6, 6,
                                                            6, 6, 1, 5,
                                                            5, 5, 5, 5,
                                                            4, 4, 3, 3,
                                                            4, 4, 3, 1});

    auto actual = clusterSize(ras, ClusterDiagonals::Exclude);
    EXPECT_RASTER_EQ(expected, actual);
}

TEST(ClusterSizeTest, clusterSizeBorderValues)
{
    const MaskedRaster<int32_t> ras(5, 4, std::vector<int32_t>{1, 2, 3, 4, 2, 9, 9, 5, 3, 9, 9, 6, 4, 9, 9, 7, 5, 6, 7, 8});

    const MaskedRaster<int32_t> expected(ras.metadata(), std::vector<int32_t>{
                                                            1, 1, 1, 1,
                                                            1, 6, 6, 1,
                                                            1, 6, 6, 1,
                                                            1, 6, 6, 1,
                                                            1, 1, 1, 1});

    auto actual = clusterSize(ras, ClusterDiagonals::Exclude);
    EXPECT_RASTER_EQ(expected, actual);
}

TEST(ClusterSizeTest, clusterSizeResultis_nodataValue)
{
    // Verify correct behavior of the following scenario:
    // Clustersize result equals the input nodata value
    // This should not become nodata

    RasterMetadata meta(5, 4);
    meta.nodata = 9.0;

    const MaskedRaster<uint8_t> ras(meta, std::vector<uint8_t>{
                                             1, 1, 1, 2,
                                             1, 1, 1, 2,
                                             1, 1, 1, 2,
                                             4, 4, 5, 2,
                                             2, 2, 2, 2});

    meta.nodata = -9999;
    const MaskedRaster<int32_t> expected(meta, std::vector<int32_t>{
                                                  9, 9, 9, 8,
                                                  9, 9, 9, 8,
                                                  9, 9, 9, 8,
                                                  2, 2, 1, 8,
                                                  8, 8, 8, 8});

    auto actual = clusterSize(ras, ClusterDiagonals::Exclude);
    EXPECT_RASTER_EQ(expected, actual);
    EXPECT_FALSE(expected.is_nodata(0, 0));
}
}
