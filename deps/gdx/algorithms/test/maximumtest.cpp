#include "gdx/test/testbase.h"

#include "gdx/algo/maximum.h"

#include <numeric>
#include <random>

namespace gdx::test {

template <class RasterType>
class MaximumTest : public TestBase<RasterType>
{
};

template <class RasterType>
class MaximumTestNoData : public TestBase<RasterType>
{
};

TYPED_TEST_CASE(MaximumTest, RasterTypes);
TYPED_TEST_CASE(MaximumTestNoData, RasterFloatTypes);

TYPED_TEST(MaximumTest, maximumValue)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    double nodata = static_cast<double>(std::numeric_limits<T>::max() / 2);

    std::vector<typename TypeParam::value_type> v(s_rows * s_cols);
    std::iota(v.begin() + 1, v.end(), this->testType(1.0));
    std::shuffle(v.begin() + 1, v.end(), std::mt19937(std::random_device()()));

    v[0] = T(nodata);

    Raster raster(RasterMetadata(s_rows, s_cols, nodata), std::move(v));
    EXPECT_EQ(this->testType((s_rows * s_cols) - 1), maximum(raster));
}

TYPED_TEST(MaximumTest, maximumValueIntegerNoData)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    const std::vector<double> v = {
        999.0, 999.0, 4.0, 4.0,
        4.0, 8.0, 4.0, 9.0,
        2.0, 4.0, 999.0, 7.0,
        4.0, 4.0, -5.0, 8.0,
        3.0, 999.0, 4.0, 999.0};

    RasterMetadata meta(5, 4);

    Raster raster1(meta, convertTo<T>(v));
    EXPECT_EQ(this->testType(999.0), maximum(raster1));

    meta.nodata = 999.0;
    Raster raster2(meta, convertTo<T>(v));
    EXPECT_EQ(this->testType(9.0), maximum(raster2));
}

TYPED_TEST(MaximumTestNoData, maximumValueHasNoData)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nan = std::numeric_limits<double>::quiet_NaN();

    const std::vector<double> v = {
        nan, nan, 4.0, 4.0,
        4.0, 8.0, 4.0, 9.0,
        2.0, 4.0, nan, 8.0,
        4.0, 4.0, 4.0, 8.0,
        3.0, nan, 4.0, -5.0};

    Raster raster(RasterMetadata(5, 4, nan), convertTo<T>(v));
    EXPECT_EQ(this->testType(9.0), maximum(raster));
}

TYPED_TEST(MaximumTestNoData, maximumValueHasNoDataAroundMax)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nan = std::numeric_limits<double>::quiet_NaN();

    const std::vector<double> v = {
        nan, nan, 4.0, 4.0,
        4.0, 8.0, nan, 9.0,
        nan, 4.0, nan, 8.0,
        4.0, 4.0, 4.0, 8.0,
        3.0, nan, 4.0, -5.0};

    Raster raster(RasterMetadata(5, 4, nan), convertTo<T>(v));
    EXPECT_EQ(this->testType(9.0), maximum(raster));
}

TYPED_TEST(MaximumTestNoData, maximumValueOnlyNodata)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    Raster raster(RasterMetadata(5, 4, 9999), 9999);
    EXPECT_THROW(maximum(raster), InvalidArgument);
}

TYPED_TEST(MaximumTestNoData, maximumMultipleRasters)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    double nod = -999;

    Raster r1(RasterMetadata(3, 3, nod), convertTo<T>(std::vector<double>({nod, nod, 4.0,
                                             4.0, 8.0, 4.0,
                                             nod, -5.0, 1.0})));

    Raster r2(RasterMetadata(3, 3, nod), convertTo<T>(std::vector<double>{nod, 1.0, 3.0,
                                             5.0, 8.0, 1.0,
                                             nod, -5.0, nod}));

    Raster expected(RasterMetadata(3, 3, nod), convertTo<T>(std::vector<double>{nod, nod, 4.0,
                                                   5.0, 8.0, 4.0,
                                                   nod, -5.0, nod}));

    EXPECT_EQ(expected, gdx::maximum<Raster>({&r1, &r2}));
}
}
