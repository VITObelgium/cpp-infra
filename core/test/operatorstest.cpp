#include "gdx/algo/cast.h"
#include "gdx/test/testbase.h"

#include <numeric>
#include <random>

namespace gdx::test {

template <class RasterType>
class OperatorsTest : public TestBase<RasterType>
{
public:
    OperatorsTest()
    {
        nodata = 100.0;
        meta   = RasterMetadata(3, 3, nodata);
    }

    RasterMetadata meta;
    double nodata;
};

TYPED_TEST_CASE(OperatorsTest, RasterTypes);

TYPED_TEST(OperatorsTest, lessEqualScalar)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    this->meta.nodata.reset();
    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  1.0, 2.0, 3.0,
                                  4.0, 4.0, 4.0,
                                  5.0, 6.0, 7.0}));

    auto resultMeta = this->meta;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              1, 1, 1,
                                              1, 1, 1,
                                              0, 0, 0});

    auto result = raster <= this->testType(4.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, lessEqualScalarHasNoData)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  nod, 1.0, 2.0,
                                  4.0, nod, 4.0,
                                  5.0, 6.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 1, 1,
                                              1, 255, 1,
                                              0, 0, 255});

    auto result = raster <= this->testType(4.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, lessEqualRaster)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    Raster raster1(3, 3, convertTo<T>(std::vector<double>{2.0, 8.0, 4.0, 4.0, 8.0, 4.0, 2.0, 4.0, -1.0}));
    Raster raster2(3, 3, convertTo<T>(std::vector<double>{1.0, 7.0, 3.0, 4.0, 8.0, 4.0, 3.0, 5.0, 0.0}));

    ResultRasterType expected(3, 3, std::vector<uint8_t>{0, 0, 0, 1, 1, 1, 1, 1, 1});

    auto result = raster1 <= raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, lessEqualRasterHasNodata)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   nod, 8.0, 4.0,
                                   4.0, nod, 4.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, nod, 4.0,
                                   3.0, 5.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255.0;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 0, 0,
                                              1, 255, 1,
                                              1, 1, 255});

    auto result = raster1 <= raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, lessScalar)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    this->meta.nodata.reset();
    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  1.0, 2.0, 3.0,
                                  4.0, 4.0, 4.0,
                                  5.0, 6.0, 7.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              1, 1, 1,
                                              0, 0, 0,
                                              0, 0, 0});

    auto result = raster < this->testType(4.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, lessScalarHasNoData)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  nod, 1.0, 2.0,
                                  4.0, nod, 4.0,
                                  5.0, 6.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 1, 1,
                                              0, 255, 0,
                                              0, 0, 255});

    auto result = raster < this->testType(4.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, lessRaster)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    this->meta.nodata.reset();
    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   2.0, 8.0, 4.0,
                                   4.0, 8.0, 4.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, 8.0, 4.0,
                                   3.0, 5.0, 0.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              0, 0, 0,
                                              0, 0, 0,
                                              1, 1, 1});

    auto result = raster1 < raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, lessRasterHasNodata)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   nod, 8.0, 4.0,
                                   4.0, nod, 4.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, nod, 4.0,
                                   3.0, 5.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 0, 0,
                                              0, 255, 0,
                                              1, 1, 255});

    auto result = raster1 < raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, greaterScalar)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    this->meta.nodata.reset();
    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  1.0, 2.0, 3.0,
                                  4.0, 4.0, 4.0,
                                  5.0, 6.0, 7.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              0, 0, 0,
                                              0, 0, 0,
                                              1, 1, 1});

    auto result = raster > this->testType(4.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, greaterScalarHasNoData)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  nod, 1.0, 2.0,
                                  4.0, nod, 4.0,
                                  5.0, 6.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 0, 0,
                                              0, 255, 0,
                                              1, 1, 255});

    auto result = raster > this->testType(4.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, greaterRaster)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    this->meta.nodata.reset();
    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   2.0, 8.0, 4.0,
                                   4.0, 8.0, 4.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, 8.0, 4.0,
                                   3.0, 5.0, 0.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              1, 1, 1,
                                              0, 0, 0,
                                              0, 0, 0});

    auto result = raster1 > raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, greaterRasterHasNodata)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   nod, 8.0, 4.0,
                                   4.0, nod, 4.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, nod, 4.0,
                                   3.0, 5.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 1, 1,
                                              0, 255, 0,
                                              0, 0, 255});

    auto result = raster1 > raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, greaterEqualScalar)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    this->meta.nodata.reset();
    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  1.0, 2.0, 3.0,
                                  4.0, 4.0, 4.0,
                                  5.0, 6.0, 7.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              0, 0, 0,
                                              1, 1, 1,
                                              1, 1, 1});

    auto result = raster >= this->testType(4.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, greaterEqualScalarHasNoData)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  nod, 1.0, 2.0,
                                  4.0, nod, 4.0,
                                  5.0, 6.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 0, 0,
                                              1, 255, 1,
                                              1, 1, 255});

    auto result = raster >= this->testType(4.0);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, greaterEqualRaster)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    this->meta.nodata.reset();
    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   2.0, 8.0, 4.0,
                                   4.0, 8.0, 4.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, 8.0, 4.0,
                                   3.0, 5.0, 0.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              1, 1, 1,
                                              1, 1, 1,
                                              0, 0, 0});

    auto result = raster1 >= raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, greaterEqualRasterHasNodata)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   nod, 8.0, 4.0,
                                   4.0, nod, 4.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, nod, 4.0,
                                   3.0, 5.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 1, 1,
                                              1, 255, 1,
                                              0, 0, 255});

    auto result = raster1 >= raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, not_equalscalar)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    this->meta.nodata.reset();
    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  1.0, 2.0, 3.0,
                                  4.0, 4.0, 4.0,
                                  5.0, 6.0, 7.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              1, 1, 1,
                                              0, 0, 0,
                                              1, 1, 1});

    auto result = raster.not_equals(this->testType(4.0));
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, not_equalscalarMatchesNodata)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  1.0, 2.0, nod,
                                  4.0, nod, 4.0,
                                  nod, 6.0, 7.0}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              1, 1, 255,
                                              1, 255, 1,
                                              255, 1, 1});

    auto result = raster.not_equals(this->testType(0.0));
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, not_equalscalarHasNoData)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  nod, 1.0, 2.0,
                                  4.0, nod, 4.0,
                                  5.0, 6.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 1, 1,
                                              0, 255, 0,
                                              1, 1, 255});

    auto result = raster.not_equals(this->testType(4.0));
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, notEqualRaster)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto resultMeta = this->meta;
    this->meta.nodata.reset();
    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   2.0, 8.0, 4.0,
                                   4.0, 8.0, 4.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, 8.0, 4.0,
                                   3.0, 5.0, 0.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              1, 1, 1,
                                              0, 0, 0,
                                              1, 1, 1});

    auto result = raster1.not_equals(raster2);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, notEqualRasterHasNodata)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   nod, 8.0, 4.0,
                                   4.0, nod, 4.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, nod, 4.0,
                                   3.0, 5.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 1, 1,
                                              0, 255, 0,
                                              1, 1, 255});

    auto result = raster1.not_equals(raster2);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, logicalAnd)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    this->meta.nodata.reset();
    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   0.0, 0.0, 0.0,
                                   4.0, 8.0, 0.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, 8.0, 0.0,
                                   0.0, 0.0, 0.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              0, 0, 0,
                                              1, 1, 0,
                                              0, 0, 0});

    auto result = raster1 && raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, logicalAndHasNodata)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   nod, 0.0, 0.0,
                                   4.0, nod, 0.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, nod, 0.0,
                                   0.0, 0.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 0, 0,
                                              1, 255, 0,
                                              0, 0, 255});

    auto result = raster1 && raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, logicalOr)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    this->meta.nodata.reset();

    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   0.0, 0.0, 0.0,
                                   4.0, 8.0, 0.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, 8.0, 0.0,
                                   0.0, 0.0, 0.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              1, 1, 1,
                                              1, 1, 0,
                                              1, 1, 1});

    auto result = raster1 || raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, logicalOrHasNodata)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   nod, 0.0, 0.0,
                                   4.0, nod, 0.0,
                                   2.0, 4.0, -1.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 7.0, 3.0,
                                   4.0, nod, 0.0,
                                   0.0, 0.0, nod}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 1, 1,
                                              1, 255, 0,
                                              1, 1, 255});

    auto result = raster1 || raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, logicalNot)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    this->meta.nodata.reset();
    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   0.0, 0.0, 0.0,
                                   4.0, 8.0, 0.0,
                                   2.0, 4.0, -1.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              1, 1, 1,
                                              0, 0, 1,
                                              0, 0, 0});

    auto result = !raster1;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, logicalNotHasNodata)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   nod, 0.0, 0.0,
                                   4.0, nod, 0.0,
                                   2.0, 4.0, -1.0}));

    auto resultMeta   = this->meta;
    resultMeta.nodata = 255;
    ResultRasterType expected(resultMeta, std::vector<uint8_t>{
                                              255, 1, 1,
                                              0, 255, 1,
                                              0, 0, 0});

    auto result = !raster1;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, minus)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    Raster raster1(3, 3, convertTo<T>(std::vector<double>{0.0, 0.0, 0.0, 4.0, 8.0, 1.0, -2.0, -4.0, -1.0}));
    Raster expected(3, 3, convertTo<T>(std::vector<double>{0.0, 0.0, 0.0, -4.0, -8.0, -1.0, 2.0, 4.0, 1.0}));

    auto result = -raster1;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, minusHasNodata)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   nod, 0.0, 0.0,
                                   4.0, nod, 1.0,
                                   -2.0, -4.0, -1.0}));

    Raster expected(this->meta, convertTo<T>(std::vector<double>{
                                    nod, 0.0, 0.0,
                                    -4.0, nod, -1.0,
                                    2.0, 4.0, 1.0}));

    auto result = -raster1;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, multiplyOperatorNodata)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster1(this->meta, convertTo<T>(std::vector<double>{
                                   nod, 2.0, 2.0,
                                   3.0, nod, 3.0,
                                   1.0, 1.0, 0.0}));

    Raster raster2(this->meta, convertTo<T>(std::vector<double>{
                                   1.0, 3.0, 3.0,
                                   3.0, nod, 3.0,
                                   3.0, 3.0, nod}));

    Raster expected(this->meta, convertTo<T>(std::vector<double>{
                                    nod, 6.0, 6.0,
                                    9.0, nod, 9.0,
                                    3.0, 3.0, nod}));

    auto result = raster1 * raster2;
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(OperatorsTest, multiplyOperatorNodataDifferentTypes)
{
    using Raster      = typename TypeParam::raster;
    using UintRaster  = decltype(raster_cast<uint8_t>(std::declval<Raster>()));
    using FloatRaster = decltype(raster_cast<float>(std::declval<Raster>()));

    uint8_t nod = 255;
    float fnod  = 255.f;

    RasterMetadata intMeta(3, 3, nod);
    RasterMetadata floatMeta(3, 3, nod);

    UintRaster raster1(intMeta, std::vector<uint8_t>{
                                    nod, 2, 2,
                                    3, nod, 3,
                                    1, 1, 0});

    FloatRaster raster2(floatMeta, std::vector<float>{
                                       1.0, 3.0, 3.0,
                                       3.0, fnod, 3.0,
                                       3.0, 3.0, fnod});

    FloatRaster expected(floatMeta, std::vector<float>{
                                        fnod, 6.0, 6.0,
                                        9.0, fnod, 9.0,
                                        3.0, 3.0, fnod});

    auto result1 = raster1 * raster2;
    EXPECT_RASTER_NEAR(expected, result1);

    auto result2 = raster2 * raster1;
    EXPECT_RASTER_NEAR(expected, result2);
}
}
