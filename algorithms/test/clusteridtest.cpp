#include "gdx/algo/clusterid.h"
#include "gdx/test/testbase.h"

#include <random>

namespace gdx::test {

TEST(ClusterIdTest, clusterId)
{
    const MaskedRaster<int32_t> ras(5, 4, std::vector<int32_t>{1, 1, 1, 1, 1, 1, 2, 3, 3, 3, 3, 3, 1, 1, 5, 5, 1, 1, 5, 1});

    const MaskedRaster<int32_t> expected(ras.metadata(), std::vector<int32_t>{
                                                            1, 1, 1, 1,
                                                            1, 1, 2, 3,
                                                            3, 3, 3, 3,
                                                            4, 4, 5, 5,
                                                            4, 4, 5, 6});

    EXPECT_RASTER_EQ(expected, clusterId(ras, ClusterDiagonals::Exclude));
}

TEST(ClusterIdTest, clusterIdBorderValues)
{
    const MaskedRaster<int32_t> ras(5, 4, std::vector<int32_t>{1, 2, 3, 4, 2, 9, 9, 5, 3, 9, 9, 6, 4, 9, 9, 7, 5, 6, 7, 8});

    const MaskedRaster<int32_t> expected(ras.metadata(), std::vector<int32_t>{
                                                            1, 2, 3, 4,
                                                            5, 6, 6, 7,
                                                            8, 6, 6, 9,
                                                            10, 6, 6, 11,
                                                            12, 13, 14, 15});

    EXPECT_RASTER_EQ(expected, clusterId(ras, ClusterDiagonals::Exclude));
}

TEST(ClusterIdTest, fuzzyclusterId)
{
    RasterMetadata meta(10, 10);
    meta.cellSize = 100;

    const MaskedRaster<float> ras(meta, std::vector<float>{
                                           1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
                                           1, 1, 0, 1, 0, 0, 1, 0, 1, 0,
                                           1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
                                           1, 0, 1, 1, 0, 0, 1, 0, 1, 0,
                                           1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                                           1, 0, 1, 0, 1, 0, 1, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

    const MaskedRaster<int32_t> expected(ras.metadata(), std::vector<int32_t>{
                                                            1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
                                                            1, 1, 0, 1, 0, 0, 2, 0, 2, 0,
                                                            1, 0, 0, 1, 0, 0, 0, 2, 0, 0,
                                                            1, 0, 1, 1, 0, 0, 2, 0, 2, 0,
                                                            1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
                                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
                                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                            0, 0, 0, 0, 0, 0, 0, 0, 4, 0,
                                                            5, 0, 6, 0, 7, 0, 8, 0, 0, 0,
                                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0});

    EXPECT_RASTER_EQ(expected, fuzzyClusterId(ras, 1.42f * static_cast<float>(meta.cellSize)));
}
}
