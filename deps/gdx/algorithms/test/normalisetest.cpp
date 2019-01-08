#include "gdx/test/testbase.h"

#include "gdx/algo/cast.h"
#include "gdx/algo/normalise.h"

#include <numeric>
#include <random>

namespace gdx::test {

template <class RasterType>
class NormaliseTest : public TestBase<RasterType>
{
public:
    RasterMetadata meta = RasterMetadata(3, 3);
};

TYPED_TEST_CASE(NormaliseTest, RasterTypes);

TYPED_TEST(NormaliseTest, normalise)
{
    using T                = typename TypeParam::value_type;
    using Raster           = typename TypeParam::raster;
    using ResultRasterType = decltype(raster_cast<uint8_t>(std::declval<Raster>()));

    if (!typeSupported<T>()) return;

    Raster raster(this->meta, convertTo<T>(std::vector<double>{
                                  0.0, 32.0, 32.0,
                                  64.0, 64.0, 64.0,
                                  96.0, 96.0, 128.0}));

    ResultRasterType expected(this->meta, std::vector<uint8_t>{
                                              0, 64, 64,
                                              127, 127, 127,
                                              191, 191, 254});

    ResultRasterType result(this->meta);
    normalise(raster, result);
    EXPECT_RASTER_EQ(expected, result);
}
}
