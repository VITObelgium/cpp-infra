#include "gdx/algo/sum.h"
#include "gdx/test/testbase.h"

#include <numeric>
#include <random>

namespace gdx::test {

template <class RasterType>
class SumTest : public TestBase<RasterType>
{
public:
    SumTest()
    {
        nodata = 100.0;
        meta   = RasterMetadata(3, 3, nodata);
    }

    RasterMetadata meta;
    double nodata;
};

TYPED_TEST_CASE(SumTest, RasterTypes);

TYPED_TEST(SumTest, sum)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  1.0, 2.0, 3.0,
                                  4.0, 4.0, 4.0,
                                  5.0, 6.0, 7.0}));

    EXPECT_EQ(36.0, sum(raster));
}

TYPED_TEST(SumTest, sumNoData)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  nod, 2.0, 3.0,
                                  4.0, nod, 4.0,
                                  5.0, 6.0, nod}));

    EXPECT_EQ(24.0, sum(raster));
}

TYPED_TEST(SumTest, sumValueOnlyNans)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    auto nod = this->nodata;

    Raster raster(this->meta, static_cast<T>(nod));
    EXPECT_EQ(0.0, sum(raster));
}
}
