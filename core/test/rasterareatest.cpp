#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "gdx/denseraster.h"
#include "gdx/maskedraster.h"
#include "gdx/rasterarea.h"
#include "gdx/sparseraster.h"
#include "gdx/test/testbase.h"
#include <vector>

namespace gdx::test {

using namespace testing;
using namespace std::string_literals;

template <class RasterType>
class RasterAreaTest : public TestBase<RasterType>
{
public:
    RasterAreaTest()
    {
        nodata = 100.0;
    }

    RasterMetadata meta;
    double nodata;
};

TYPED_TEST_CASE(RasterAreaTest, RasterTypes);

TYPED_TEST(RasterAreaTest, cellNeighboursSquare)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    // clang-format off
    auto data = convertTo<T>(std::vector<double>{{
        -1,  1,  2,  3,  4,
         5,  6,  7,  8, -1,
        10, -1, -1, -1, 14,
        15, -1, -1, -1, 19,
    }});
    // clang-format on

    Raster raster(RasterMetadata(4, 5, -1), data);

    std::vector<T> result;
    auto area = neighbouring_cells_square(raster, Cell(1, 1), 1);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({1, 2, 5, 7, 10})));
    result.clear();

    area = neighbouring_cells_square(raster, Cell(0, 0), 1);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({1, 5, 6})));
    result.clear();

    area = neighbouring_cells_square(raster, Cell(0, 3), 1);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({2, 4, 7, 8})));
    result.clear();

    area = neighbouring_cells_square(raster, Cell(0, 4), 1);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({3, 8})));
    result.clear();

    area = neighbouring_cells_square(raster, Cell(3, 2), 1);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_TRUE(result.empty());
    result.clear();

    area = neighbouring_cells_square(raster, Cell(3, 4), 1);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({14})));
    result.clear();

    area = neighbouring_cells_square(raster, Cell(3, 0), 1);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({10})));
    result.clear();

    area = neighbouring_cells_square(raster, Cell(1, 2), 2);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({1, 2, 3, 4, 5, 6, 8, 10, 14, 15, 19})));
    result.clear();
}

TYPED_TEST(RasterAreaTest, cellNeighboursSquareIteratorCell)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    Raster raster(RasterMetadata(4, 5, -1), convertTo<T>(std::vector<double>(4 * 5, 9)));

    std::vector<Cell> result;
    auto area = neighbouring_cells_square(raster, Cell(1, 1), 1);
    for (auto iter = area.begin(); iter != area.end(); ++iter) {
        result.push_back(iter.cell());
    }

    EXPECT_THAT(result, ContainerEq(std::vector<Cell>({{0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 2}, {2, 0}, {2, 1}, {2, 2}})));
}

TYPED_TEST(RasterAreaTest, cellNeighboursCircular)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    // clang-format off
    auto data = convertTo<T>(std::vector<double>{{
        -1,  1,  2,  3,  4, 0,
         5,  6,  7,  8, -1, 0,
        10, -1,  0, -1, 14, 0,
        15, -1, -1, 18, 19, 0,
        20, 21, 22, 23, 24, 0,
    }});
    // clang-format on

    Raster raster(RasterMetadata(5, 6, -1), data);

    // radius = 1
    std::vector<T> result;
    auto area = neighbouring_cells_circular(raster, Cell(2, 2), 1);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({7})));
    result.clear();

    // radius = 2
    area = neighbouring_cells_circular(raster, Cell(2, 2), 2);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({2, 6, 7, 8, 10, 14, 18, 22})));
    result.clear();

    // radius = 2, edge case
    area = neighbouring_cells_circular(raster, Cell(1, 1), 2);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({1, 2, 5, 7, 8, 10, 0})));
    result.clear();

    // Top left, big radius
    area = neighbouring_cells_circular(raster, Cell(0, 0), 100);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({1, 2, 3, 4, 0, 5, 6, 7, 8, 0, 10, 0, 14, 0, 15, 18, 19, 0, 20, 21, 22, 23, 24, 0})));
    result.clear();

    // Bottom right, big radius
    area = neighbouring_cells_circular(raster, Cell(4, 5), 100);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({1, 2, 3, 4, 0, 5, 6, 7, 8, 0, 10, 0, 14, 0, 15, 18, 19, 0, 20, 21, 22, 23, 24})));
    result.clear();
}

TYPED_TEST(RasterAreaTest, rasterSubArea)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    // clang-format off
    auto data = convertTo<T>(std::vector<double>{{
        -1,  1,  2,  3,  4,
         5,  6,  7,  8, -1,
        10, -1, -1, -1, 14,
        15, -1, -1, -1, 19,
    }});
    // clang-format on

    Raster raster(RasterMetadata(4, 5, -1), data);

    std::vector<T> result;
    // Center area
    auto area = sub_area(raster, Cell(1, 1), 2, 3);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({6, 7, 8})));
    result.clear();

    // Full area
    area = sub_area(raster, Cell(0, 0), 4, 5);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({1, 2, 3, 4, 5, 6, 7, 8, 10, 14, 15, 19})));
    result.clear();

    // Top left area
    area = sub_area(raster, Cell(0, 0), 3, 2);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({1, 5, 6, 10})));
    result.clear();

    // Bottom right area, too large
    area = sub_area(raster, Cell(2, 3), 4, 4);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({14, 19})));
    result.clear();

    // Single cell
    area = sub_area(raster, Cell(1, 2), 1, 1);
    std::copy(area.begin(), area.end(), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>({7})));
    result.clear();
}

TYPED_TEST(RasterAreaTest, fillRasterSubArea)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    // clang-format off
    auto data = convertTo<T>(std::vector<double>{{
        -1,  1,  2,  3,  4,
         5,  6,  7,  8, -1,
        10, 11, -1, 13, 14,
        15, -1, -1, -1, 19,
    }});

    Raster expected(RasterMetadata(4, 5, -1), convertTo<T>(std::vector<double>{{
        -1,  1,  2,  3,  4,
         5,  0,  0,  0, -1,
        10,  0, -1,  0, 14,
        15, -1, -1, -1, 19,
    }}));
    // clang-format on

    Raster raster(RasterMetadata(4, 5, -1), data);

    std::vector<T> result;

    auto area = sub_area(raster, Cell(1, 1), 2, 3);
    std::fill(area.begin(), area.end(), T(0));
    EXPECT_RASTER_EQ(expected, raster);
}
}
