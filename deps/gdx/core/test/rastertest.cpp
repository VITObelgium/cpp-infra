#include "gdx/test/testbase.h"

#include "gdx/algo/cast.h"
#include "gdx/raster.h"

#include <type_traits>

namespace gdx::test {

template <class RasterType>
class RasterTest : public TestBase<RasterType>
{
};

template <class RasterType>
class RasterTestIntTypes : public TestBase<RasterType>
{
};

template <class RasterType>
class RasterTestFloatTypes : public TestBase<RasterType>
{
};

TYPED_TEST_CASE(RasterTest, RasterTypes);
TYPED_TEST_CASE(RasterTestIntTypes, RasterIntTypes);
TYPED_TEST_CASE(RasterTestFloatTypes, RasterFloatTypes);

TEST(RasterTest, multiplyOperatorDifferentTypes)
{
    MaskedRaster<float> floatRas(s_rows, s_cols, 2.f);
    MaskedRaster<uint8_t> intRas(s_rows, s_cols, 3);
    MaskedRaster<float> expected(s_rows, s_cols, 6.f);

    auto res = floatRas * intRas;

    EXPECT_TRUE(expected.tolerant_equal_to(res));
}

TYPED_TEST(RasterTest, sumOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterType r1, r2, expected;
    r1.resize_and_fill(3, 3, this->testType(6.0));
    r2.resize_and_fill(3, 3, this->testType(3.0));
    expected.resize_and_fill(3, 3, this->testType(9.0));

    auto res = r1 + r2;

    EXPECT_EQ(expected, res);
}

TYPED_TEST(RasterTest, subtractOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterType r1, r2, expected;
    r1.resize_and_fill(s_rows, s_cols, this->testType(4.f));
    r2.resize_and_fill(s_rows, s_cols, this->testType(5.f));
    expected.resize_and_fill(s_rows, s_cols, this->testType(-1.0));

    auto res = r1 - r2;

    EXPECT_EQ(r1.size(), res.size());
    EXPECT_EQ(r1.rows(), res.rows());
    EXPECT_EQ(r1.cols(), res.cols());

    EXPECT_EQ(expected, res);
}

TYPED_TEST(RasterTest, multiplyOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterType r1, r2, expected;
    r1.resize_and_fill(s_rows, s_cols, this->testType(4.0));
    r2.resize_and_fill(s_rows, s_cols, this->testType(5.0));
    expected.resize_and_fill(s_rows, s_cols, this->testType(20.0));

    auto res = r1 * r2;

    EXPECT_EQ(r1.size(), res.size());
    EXPECT_EQ(r1.rows(), res.rows());
    EXPECT_EQ(r1.cols(), res.cols());

    EXPECT_EQ(expected, res);
}

TYPED_TEST(RasterTest, divideOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterType r1, r2;
    r1.resize_and_fill(s_rows, s_cols, this->testType(16.0));
    r2.resize_and_fill(s_rows, s_cols, this->testType(8.0));
    decltype(r1 / r2) expected(s_rows, s_cols, 2.f);

    auto res = r1 / r2;

    EXPECT_EQ(r1.size(), res.size());
    EXPECT_EQ(r1.rows(), res.rows());
    EXPECT_EQ(r1.cols(), res.cols());

    EXPECT_RASTER_EQ(expected, res);
}

TYPED_TEST(RasterTest, equalTo)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterType r1, r2, expected;
    r1.resize_and_fill(s_rows, s_cols, this->testType(4.0));
    r2.resize_and_fill(s_rows, s_cols, this->testType(2.0));

    EXPECT_RASTER_NE(r1, r2);

    r2.fill(this->testType(4.0));

    EXPECT_RASTER_EQ(r1, r2);
}

TYPED_TEST(RasterTestIntTypes, addRastersNodata)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterMetadata meta(2, 3, -1.0);

    RasterType raster1(meta, convertTo<T>(std::vector<double>{
                                 -1.0, -1.0, 2.0,
                                 4.0, -1.0, 6.0}));

    auto metaDifferentNodata   = meta;
    metaDifferentNodata.nodata = -2.0;

    RasterType raster2(metaDifferentNodata, convertTo<T>(std::vector<double>{
                                                1.0, -2.0, 3.0,
                                                1.0, -2.0, -2.0}));

    RasterType expected(meta, convertTo<T>(std::vector<double>{
                                  -1.0, -1.0, 5.0,
                                  5.0, -1.0, -1.0}));

    EXPECT_RASTER_EQ(expected, raster1 + raster2);
}

TYPED_TEST(RasterTestIntTypes, subtractRastersNodata)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterMetadata meta(2, 3);
    meta.nodata = -1.0;

    RasterType raster1(meta, convertTo<T>(std::vector<double>{
                                 -1.0, -1.0, 8.0,
                                 4.0, -1.0, 6.0}));

    RasterType raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, -1.0, 3.0,
                                 1.0, -1.0, -1.0}));

    RasterType expected(meta, convertTo<T>(std::vector<double>{
                                  -1.0, -1.0, 5.0,
                                  3.0, -1.0, -1.0}));

    EXPECT_EQ(expected, raster1 - raster2);
}

TYPED_TEST(RasterTestIntTypes, multiplyRastersNodata)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterMetadata meta(2, 3);
    meta.nodata = -1.0;

    RasterType raster1(meta, convertTo<T>(std::vector<double>{
                                 -1.0, -1.0, 8.0,
                                 4.0, -1.0, 6.0}));

    RasterType raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, -1.0, 3.0,
                                 1.0, -1.0, -1.0}));

    RasterType expected(meta, convertTo<T>(std::vector<double>{
                                  -1.0, -1.0, 24.0,
                                  4.0, -1.0, -1.0}));

    EXPECT_EQ(expected, raster1 * raster2);
}

template <typename T, typename Raster>
static auto cast(const Raster&)
{
}

TYPED_TEST(RasterTestIntTypes, divideRastersNodata)
{
    using T                = typename TypeParam::value_type;
    using RasterType       = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<float>(std::declval<RasterType>()));
    if (!typeSupported<T>()) return;

    RasterMetadata meta(2, 3);
    meta.nodata = -1.0;

    RasterType raster1(meta, convertTo<T>(std::vector<double>{
                                 -1.0, -1.0, 8.0,
                                 4.0, -1.0, 6.0}));

    RasterType raster2(meta, convertTo<T>(std::vector<double>{
                                 1.0, -1.0, 4.0,
                                 1.0, -1.0, -1.0}));

    ResultRasterType expected(meta, convertTo<float>(std::vector<double>{
                                        -1.0, -1.0, 2.0,
                                        4.0, -1.0, -1.0}));

    auto res = raster1 / raster2;
    static_assert(std::is_same_v<ResultRasterType, decltype(res)>, "Types should match");
    EXPECT_RASTER_EQ(expected, res);
}

TYPED_TEST(RasterTest, subtractScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterType raster(5, 4, convertTo<T>(std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0}));
    RasterType expected(5, 4, convertTo<T>(std::vector<double>{-1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0, -1.0}));

    auto result = raster - this->testType(1.0);
    EXPECT_RASTER_EQ(expected, result);
}

TYPED_TEST(RasterTestIntTypes, subtractScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterMetadata meta(5, 4);
    meta.nodata = -1.0;

    RasterType raster(meta, convertTo<T>(std::vector<double>{
                                -1.0, -1.0, 2.0, 3.0,
                                4.0, 5.0, 6.0, 7.0,
                                8.0, -1.0, -1.0, 8.0,
                                7.0, 6.0, 5.0, 4.0,
                                3.0, 2.0, -1.0, -1.0}));

    RasterType expected(meta, convertTo<T>(std::vector<double>{
                                  -1.0, -1.0, 1.0, 2.0,
                                  3.0, 4.0, 5.0, 6.0,
                                  7.0, -1.0, -1.0, 7.0,
                                  6.0, 5.0, 4.0, 3.0,
                                  2.0, 1.0, -1.0, -1.0}));

    auto result = raster - this->testType(1.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(RasterTestFloatTypes, subtractScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    constexpr auto nan = -1.0;

    using RasterType = typename TypeParam::raster;

    RasterMetadata meta(5, 4);
    meta.nodata = -1.0;

    RasterType raster(meta, convertTo<T>(std::vector<double>{
                                nan, nan, 2.0, 3.0,
                                4.0, 5.0, 6.0, 7.0,
                                8.0, nan, nan, 8.0,
                                7.0, 6.0, 5.0, 4.0,
                                3.0, 2.0, nan, nan}));

    RasterType expected(meta, convertTo<T>(std::vector<double>{
                                  nan, nan, 1.0, 2.0,
                                  3.0, 4.0, 5.0, 6.0,
                                  7.0, nan, nan, 7.0,
                                  6.0, 5.0, 4.0, 3.0,
                                  2.0, 1.0, nan, nan}));

    auto result = raster - this->testType(1.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(RasterTestFloatTypes, subtractScalarOperatorScalarFirst)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    constexpr auto nod = -1.0;

    RasterMetadata meta(5, 4);
    meta.nodata = -1.0;

    RasterType raster(meta, convertTo<T>(std::vector<double>{
                                nod, nod, 2.0, 3.0,
                                4.0, 5.0, 6.0, 7.0,
                                8.0, nod, nod, 8.0,
                                7.0, 6.0, 5.0, 4.0,
                                3.0, 2.0, nod, nod}));

    RasterType expected(meta, convertTo<T>(std::vector<double>{
                                  nod, nod, 8.0, 7.0,
                                  6.0, 5.0, 4.0, 3.0,
                                  2.0, nod, nod, 2.0,
                                  3.0, 4.0, 5.0, 6.0,
                                  7.0, 8.0, nod, nod}));

    auto result = this->testType(10.0) - raster;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(RasterTestFloatTypes, divideScalarOperatorScalarFirst)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    constexpr auto nod = -1.0;

    RasterMetadata meta(5, 4);
    meta.nodata = -1.0;

    RasterType raster(meta, convertTo<T>(std::vector<double>{
                                nod, nod, 2.0, 3.0,
                                4.0, 4.0, 6.0, 24.0,
                                8.0, nod, nod, 8.0,
                                48.0, 6.0, 12.0, 4.0,
                                3.0, 8.0, nod, nod}));

    auto result = this->testType(24) / raster;

    decltype(result) expected(meta, convertTo<typename decltype(result)::value_type>(std::vector<double>{
                                        nod, nod, 12.0, 8.0,
                                        6.0, 6.0, 4.0, 1.0,
                                        3.0, nod, nod, 3.0,
                                        0.5, 4.0, 2.0, 6.0,
                                        8.0, 3.0, nod, nod}));

    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(RasterTestIntTypes, divideByZero)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    using ResultType = decltype(raster_cast<float>(std::declval<RasterType>()));
    if (!typeSupported<T>()) return;

    // input rasters do not have no data, nan is used as nodata using RasterType = typename TypeParam::raster;

    RasterType raster1(RasterMetadata(5, 4), 0);
    RasterType raster2(RasterMetadata(5, 4), 0);
    ResultType expected(RasterMetadata(5, 4, 255), 255);
    auto result = raster1 / raster2;
    EXPECT_RASTER_EQ(expected, result);
    EXPECT_TRUE(std::isnan(*result.nodata()));

    // input rasters do have no data, that value is used
    raster1.set_nodata(99);
    raster2.set_nodata(99);
    expected.set_nodata(99);
    result = raster1 / raster2;
    EXPECT_RASTER_EQ(expected, result);
    EXPECT_EQ(99.0, result.nodata());
}

TYPED_TEST(RasterTestIntTypes, divideIntRasterBecomesFloat)
{
    using RasterType = typename TypeParam::raster;
    using ResultType = decltype(raster_cast<float>(std::declval<RasterType>()));

    RasterMetadata meta(5, 4);
    RasterType raster1(meta, 0);
    RasterType raster2(meta, 1);

    static_assert(std::is_same_v<decltype(raster1 / raster2), ResultType>, "Integer division should become float");

    meta.nodata = -1.0;
}

TYPED_TEST(RasterTest, addScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterType raster(5, 4, convertTo<T>(std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0}));
    RasterType expected(5, 4, convertTo<T>(std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0}));

    auto result = raster + this->testType(1.0);
    EXPECT_RASTER_EQ(expected, result);
}

TYPED_TEST(RasterTestIntTypes, addScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterMetadata meta(5, 4);
    meta.nodata = -1.0;

    RasterType raster(meta, convertTo<T>(std::vector<double>{
                                -1.0, -1.0, 2.0, 3.0,
                                4.0, 5.0, 6.0, 7.0,
                                8.0, -1.0, -1.0, 8.0,
                                7.0, 6.0, 5.0, 4.0,
                                3.0, 2.0, -1.0, -1.0}));

    RasterType expected(meta, convertTo<T>(std::vector<double>{
                                  -1.0, -1.0, 3.0, 4.0,
                                  5.0, 6.0, 7.0, 8.0,
                                  9.0, -1.0, -1.0, 9.0,
                                  8.0, 7.0, 6.0, 5.0,
                                  4.0, 3.0, -1.0, -1.0}));

    auto result = raster + this->testType(1.0);

    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(RasterTestFloatTypes, addScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    constexpr auto nod = -1.0;

    RasterType raster(RasterMetadata(5, 4, nod), convertTo<T>(std::vector<double>{
                                                     nod, nod, 2.0, 3.0,
                                                     4.0, 5.0, 6.0, 7.0,
                                                     8.0, nod, nod, 8.0,
                                                     7.0, 6.0, 5.0, 4.0,
                                                     3.0, 2.0, nod, nod}));

    RasterType expected(RasterMetadata(5, 4, nod), convertTo<T>(std::vector<double>{
                                                       nod, nod, 3.0, 4.0,
                                                       5.0, 6.0, 7.0, 8.0,
                                                       9.0, nod, nod, 9.0,
                                                       8.0, 7.0, 6.0, 5.0,
                                                       4.0, 3.0, nod, nod}));

    auto result = raster + this->testType(1.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(RasterTest, multiplyScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterType raster(5, 4, convertTo<T>(std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0}));
    RasterType expected(5, 4, convertTo<T>(std::vector<double>{0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 18.0, 16.0, 14.0, 12.0, 10.0, 8.0, 6.0, 4.0, 2.0, 0.0}));

    auto result = raster * this->testType(2.0);
    EXPECT_RASTER_EQ(expected, result);
}

TYPED_TEST(RasterTestIntTypes, multiplyScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    constexpr auto nod = -1.0;

    RasterMetadata meta(5, 4, nod);
    using RasterType = typename TypeParam::raster;

    RasterType raster(meta, convertTo<T>(std::vector<double>{
                                nod, nod, 2.0, 3.0,
                                4.0, 5.0, 6.0, 7.0,
                                8.0, nod, nod, 8.0,
                                7.0, 6.0, 5.0, 4.0,
                                3.0, 2.0, nod, nod}));

    RasterType expected(meta, convertTo<T>(std::vector<double>{
                                  nod, nod, 4.0, 6.0,
                                  8.0, 10.0, 12.0, 14.0,
                                  16.0, nod, nod, 16.0,
                                  14.0, 12.0, 10.0, 8.0,
                                  6.0, 4.0, nod, nod}));

    auto result = raster * this->testType(2.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(RasterTestFloatTypes, multiplyScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    constexpr auto nod = -1.0;
    RasterMetadata meta(5, 4, nod);

    RasterType raster(meta, convertTo<T>(std::vector<double>{
                                nod, nod, 2.0, 3.0,
                                4.0, 5.0, 6.0, 7.0,
                                8.0, nod, nod, 8.0,
                                7.0, 6.0, 5.0, 4.0,
                                3.0, 2.0, nod, nod}));

    RasterType expected(meta, convertTo<T>(std::vector<double>{
                                  nod, nod, 4.0, 6.0,
                                  8.0, 10.0, 12.0, 14.0,
                                  16.0, nod, nod, 16.0,
                                  14.0, 12.0, 10.0, 8.0,
                                  6.0, 4.0, nod, nod}));

    auto result = raster * this->testType(2.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(RasterTest, divideScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterType raster(5, 4, convertTo<T>(std::vector<double>{0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 18.0, 16.0, 14.0, 12.0, 10.0, 8.0, 6.0, 4.0, 2.0, 0.0}));
    RasterType expected(5, 4, convertTo<T>(std::vector<double>{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0}));

    auto result = raster / this->testType(2.0);
    EXPECT_RASTER_EQ(expected, result);
}

TYPED_TEST(RasterTestIntTypes, divideScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    constexpr auto nod = -1.0;
    RasterMetadata meta(5, 4, nod);

    RasterType raster(meta, convertTo<T>(std::vector<double>{
                                nod, nod, 4.0, 6.0,
                                8.0, 10.0, 12.0, 14.0,
                                16.0, nod, nod, 16.0,
                                14.0, 12.0, 10.0, 8.0,
                                6.0, 4.0, nod, nod}));

    RasterType expected(meta, convertTo<T>(std::vector<double>{
                                  nod, nod, 2.0, 3.0,
                                  4.0, 5.0, 6.0, 7.0,
                                  8.0, nod, nod, 8.0,
                                  7.0, 6.0, 5.0, 4.0,
                                  3.0, 2.0, nod, nod}));

    auto result = raster / this->testType(2.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(RasterTestFloatTypes, divideScalarOperator)
{
    using T          = typename TypeParam::value_type;
    using RasterType = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    constexpr auto nod = -1.0;
    RasterMetadata meta(5, 4, nod);

    RasterType raster(meta, convertTo<T>(std::vector<double>{
                                nod, nod, 4.0, 6.0,
                                8.0, 10.0, 12.0, 14.0,
                                16.0, nod, nod, 16.0,
                                14.0, 12.0, 10.0, 8.0,
                                6.0, 4.0, nod, nod}));

    RasterType expected(meta, convertTo<T>(std::vector<double>{
                                  nod, nod, 2.0, 3.0,
                                  4.0, 5.0, 6.0, 7.0,
                                  8.0, nod, nod, 8.0,
                                  7.0, 6.0, 5.0, 4.0,
                                  3.0, 2.0, nod, nod}));

    auto result = raster / this->testType(2.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TEST(RasterTest, multiplyDifferentTypes)
{
    MaskedRaster<float> r1, expected;
    MaskedRaster<int32_t> r2;

    r1.resize_and_fill(3, 3, 4.f);
    r2.resize_and_fill(3, 3, 5);
    expected.resize_and_fill(3, 3, 20.f);

    EXPECT_RASTER_EQ(expected, r1 * r2);
}

}
