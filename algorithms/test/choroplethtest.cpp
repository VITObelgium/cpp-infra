#include "gdx/test/testbase.h"

#include "gdx/algo/choropleth.h"

#include <numeric>
#include <random>

namespace gdx::test {

template <class RasterType>
class ChoroPlethTest : public TestBase<RasterType>
{
public:
    RasterMetadata meta = RasterMetadata(3, 3);
};

TYPED_TEST_CASE(ChoroPlethTest, RasterTypes);

TYPED_TEST(ChoroPlethTest, choroplethSum)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  0.0, 1.0, 2.0,
                                  3.0, 4.0, 5.0,
                                  6.0, 7.0, 8.0}));

    Raster areas(this->meta, convertTo<T>(std::vector<double>{
                                 1.0, 1.0, 2.0,
                                 1.0, 1.0, 2.0,
                                 3.0, 3.0, 3.0}));

    Raster expected(this->meta, convertTo<T>(std::vector<double>{
                                    8.0, 8.0, 7.0,
                                    8.0, 8.0, 7.0,
                                    21.0, 21.0, 21.0}));

    Raster result(this->meta);
    choroplethSum(raster, areas, result);
    EXPECT_RASTER_EQ(expected, result);
}

TYPED_TEST(ChoroPlethTest, choroplethAvg)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  0.0, 1.0, 2.0,
                                  3.0, 4.0, 5.0,
                                  6.0, 7.0, 8.0}));

    Raster areas(this->meta, convertTo<T>(std::vector<double>{
                                 1.0, 1.0, 2.0,
                                 1.0, 1.0, 2.0,
                                 3.0, 3.0, 3.0}));

    Raster expected(this->meta, convertTo<T>(std::vector<double>{
                                    2.0, 2.0, 3.5,
                                    2.0, 2.0, 3.5,
                                    7.0, 7.0, 7.0}));

    Raster result(this->meta);
    choroplethAverage(raster, areas, result);
    EXPECT_RASTER_EQ(expected, result);
}
}
