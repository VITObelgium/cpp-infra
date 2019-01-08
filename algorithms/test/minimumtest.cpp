#include "gdx/test/testbase.h"

#include "gdx/algo/minimum.h"

#include <numeric>
#include <random>

namespace gdx::test {

template <class RasterType>
class MinimumTest : public TestBase<RasterType>
{
};

template <class RasterType>
class MinimumTestNoData : public TestBase<RasterType>
{
};

TYPED_TEST_CASE(MinimumTest, RasterTypes);
TYPED_TEST_CASE(MinimumTestNoData, RasterFloatTypes);

TYPED_TEST(MinimumTest, minimumValue)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nodata = static_cast<double>(std::numeric_limits<T>::lowest());

    std::vector<T> v(s_rows * s_cols);
    std::iota(v.begin() + 1, v.end(), this->testType(5.0));
    std::shuffle(v.begin() + 1, v.end(), std::mt19937(std::random_device()()));

    v[0] = T(nodata);

    Raster raster(RasterMetadata(s_rows, s_cols, nodata), v);

    EXPECT_EQ(this->testType(5.0), minimum(raster));
    EXPECT_EQ(this->testType(10.0), minimum(raster * 2));
}

TYPED_TEST(MinimumTest, minimumValueIntegerNoData)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    const std::vector<double> v = {
        -999.0, -999.0, 4.0, 4.0,
        4.0, 8.0, 4.0, 9.0,
        2.0, 4.0, -999.0, 7.0,
        4.0, 4.0, -5.0, 8.0,
        3.0, -999.0, 4.0, -999.0};

    RasterMetadata meta(5, 4);

    Raster raster1(meta, convertTo<T>(v));
    EXPECT_EQ(this->testType(-999.0), minimum(raster1));

    meta.nodata = -999.0;
    Raster raster2(meta, convertTo<T>(v));
    EXPECT_EQ(this->testType(-5.0), minimum(raster2));
}

TYPED_TEST(MinimumTestNoData, minimumValueHasNoData)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nan = std::numeric_limits<double>::quiet_NaN();

    const std::vector<double> v = {
        nan, nan, 4.0, 4.0,
        4.0, 8.0, 4.0, 9.0,
        2.0, 4.0, nan, 7.0,
        4.0, 4.0, 4.0, 8.0,
        3.0, nan, 4.0, -5.0};

    Raster raster(RasterMetadata(5, 4, nan), convertTo<T>(v));
    EXPECT_EQ(this->testType(-5.f), minimum(raster));
}

TYPED_TEST(MinimumTestNoData, minimumValueNoDataAroundMin)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nan = std::numeric_limits<double>::quiet_NaN();

    const std::vector<double> v = {
        nan, nan, 4.0, 4.0,
        4.0, 8.0, 4.0, 9.0,
        nan, -5.0, nan, 7.0,
        4.0, 4.0, 4.0, 8.0,
        3.0, nan, 4.0, 5.0};

    Raster raster(RasterMetadata(5, 4, nan), convertTo<T>(v));
    EXPECT_EQ(this->testType(-5.f), minimum(raster));
}

TYPED_TEST(MinimumTestNoData, minimumValueOnlyNodata)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    Raster raster(RasterMetadata(5, 4, 9999), 9999);
    EXPECT_THROW(minimum(raster), InvalidArgument) << minimum(raster);
}

TYPED_TEST(MinimumTestNoData, minimumValueOnlyNaNNodata)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    Raster raster(RasterMetadata(5, 4, Raster::NaN), Raster::NaN);
    EXPECT_THROW(minimum(raster), InvalidArgument) << minimum(raster);
}

TYPED_TEST(MinimumTestNoData, minimumMultipleRasters)
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

    Raster expected(RasterMetadata(3, 3, nod), convertTo<T>(std::vector<double>{nod, nod, 3.0,
                                                   4.0, 8.0, 1.0,
                                                   nod, -5.0, nod}));

    EXPECT_RASTER_EQ(expected, gdx::minimum<Raster>({&r1, &r2}));
}

TYPED_TEST(MinimumTestNoData, minmaxHasNoData)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nan = std::numeric_limits<double>::quiet_NaN();

    const std::vector<double> v = {
        nan, nan, 4.0, 4.0,
        4.0, 8.0, 4.0, 9.0,
        2.0, 4.0, nan, 7.0,
        4.0, 4.0, 4.0, 8.0,
        3.0, nan, 4.0, -5.0};

    Raster raster(RasterMetadata(5, 4, nan), convertTo<T>(v));
    std::pair<T, T> expected(T(-5), T(9));
    EXPECT_EQ(expected, minmax(raster));
}

TYPED_TEST(MinimumTestNoData, minmaxCellHasNoData)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nan = std::numeric_limits<double>::quiet_NaN();

    const std::vector<double> v = {
        nan, nan, 4.0, 4.0,
        4.0, 8.0, 4.0, 9.0,
        2.0, 4.0, nan, 7.0,
        4.0, 4.0, 4.0, 8.0,
        3.0, nan, 4.0, -5.0};

    Raster raster(RasterMetadata(5, 4, nan), convertTo<T>(v));
    auto [minCell, maxCell] = minmax_cell(raster);
    EXPECT_EQ(T(-5), raster[minCell]);
    EXPECT_EQ(T(9), raster[maxCell]);

    EXPECT_EQ(Cell(4, 3), minCell);
    EXPECT_EQ(Cell(1, 3), maxCell);
}
}
