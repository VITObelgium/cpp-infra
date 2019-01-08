#include "gdx/test/testbase.h"

#include "gdx/algo/mathoperations.h"

#include <numeric>
#include <random>

namespace gdx::test {

template <class RasterType>
class MathOperationsTest : public TestBase<RasterType>
{
};

template <class RasterType>
class MathOperationsTestNoData : public TestBase<RasterType>
{
};

TYPED_TEST_CASE(MathOperationsTest, RasterTypes);

TYPED_TEST(MathOperationsTest, abs)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nan = std::numeric_limits<double>::quiet_NaN();

    const std::vector<double> v = {
        nan, 1.0, -1.0,
        -4.0, nan, -2.0,
        0.0, 2.0, nan};

    const std::vector<double> exp = {
        nan, 1.0, 1.0,
        4.0, nan, 2.0,
        0.0, 2.0, nan};

    Raster raster(RasterMetadata(3, 3, nan), convertTo<T>(v));
    Raster expected(RasterMetadata(3, 3, nan), convertTo<T>(exp));
    EXPECT_RASTER_EQ(expected, gdx::abs(raster));
}

TYPED_TEST(MathOperationsTest, clip)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nan = std::numeric_limits<double>::quiet_NaN();

    const std::vector<double> input = {
        nan, 0.0, -1.0,
        1.0, nan, 2.0,
        3.0, 4.0, nan};

    const std::vector<double> expectedData = {
        nan, 1.0, 1.0,
        1.0, nan, 2.0,
        3.0, 3.0, nan};

    Raster raster(RasterMetadata(3, 3, nan), convertTo<T>(input));
    Raster expected(RasterMetadata(3, 3, nan), convertTo<T>(expectedData));
    EXPECT_RASTER_EQ(expected, gdx::clip(raster, T(1), T(3)));
}
}
