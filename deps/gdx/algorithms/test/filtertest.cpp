#include "gdx/rasterarea.h"

#include "gdx/algo/filter.h"
#include "gdx/algo/sum.h"
#include "gdx/test/testbase.h"

#include <random>

namespace gdx::test {

template <class RasterType>
class FilterTest : public TestBase<RasterType>
{
};

TYPED_TEST_CASE(FilterTest, RasterFloatTypes);

TYPED_TEST(FilterTest, filterConstant)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    // clang-format off
    const Raster ras(RasterMetadata(2, 3, -1.0), convertTo<T>(std::vector<double>({
        1, 1, 1,
        1, 1, 2,
    })));

    const Raster expected(ras.metadata(), convertTo<T>(std::vector<double>({
        1.0/3.0 + 1.0/4.0 + 1.0/3.0, 1.0/3.0 + 1.0/4.0 + 1.0/3.0 + 1.0 / 4.0, 1.0 / 4.0 + 1.0 / 3.0 + 2.0 / 3.0,
        1.0/3.0 + 1.0/4.0 + 1.0/3.0, 1.0/3.0 + 1.0/4.0 + 2.0/3.0 + 1.0 / 4.0, 1.0 / 4.0 + 2.0 / 3.0 + 1.0 / 3.0,
    })));
    // clang-format on

    auto actual = filter<Raster>(ras, FilterMode::Constant, 1);
    EXPECT_RASTER_NEAR(expected, actual);
    EXPECT_NEAR(sum(ras), sum(actual), 10e-5);
}

TYPED_TEST(FilterTest, averageFilterSquare)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    // clang-format off
    const Raster ras(RasterMetadata(3, 4, -1.0), convertTo<T>(std::vector<double>({
        1,  2, -1,  4,
        5,  6,  7,  8,
       -1, 10, 11, -1,
    })));

    const Raster expected(ras.metadata(), convertTo<T>(std::vector<double>({
        14/4.0, 21/5.0, 27/5.0, 19/3.0,
        24/5.0, 42/7.0, 48/7.0, 30/4.0,
        21/3.0, 39/5.0, 42/5.0, 26/3.0,
    })));
    // clang-format on

    auto actual = average_filter_square<Raster>(ras, 1);
    EXPECT_RASTER_NEAR(expected, actual);
}

}
