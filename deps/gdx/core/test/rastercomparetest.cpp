#include "gdx/test/testbase.h"

#include "gdx/algo/cast.h"
#include "gdx/rastercompare.h"

#include <numeric>
#include <random>

namespace gdx::test {

template <class RasterType>
class CompareTest : public TestBase<RasterType>
{
protected:
    CompareTest()
    {
        nodata = 100.0;
        meta   = RasterMetadata(3, 3, nodata);
    }

    RasterMetadata meta;
    double nodata;
};

TYPED_TEST_CASE(CompareTest, RasterTypes);

TYPED_TEST(CompareTest, equalScalar)
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
                                              0, 0, 0});

    auto result = equals(raster, this->testType(4.0));
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(CompareTest, equalScalarHasNoData)
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
                                              0, 0, 255});

    auto result = equals(raster, this->testType(4.0));
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(CompareTest, equalRaster)
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
                                              1, 1, 1,
                                              0, 0, 0});

    auto result = equals(raster1, raster2);
    EXPECT_RASTER_NEAR(expected, result);
}

TYPED_TEST(CompareTest, equalRasterHasNodata)
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
                                              1, 255, 1,
                                              0, 0, 255});

    auto result = equals(raster1, raster2);
    EXPECT_RASTER_NEAR(expected, result);
}
}
