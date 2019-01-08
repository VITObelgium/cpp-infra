#include "gdx/algo/suminbuffer.h"
#include "gdx/test/testbase.h"

namespace gdx::test {

template <class RasterType>
class SumInBufferTest : public TestBase<RasterType>
{
};

TYPED_TEST_CASE(SumInBufferTest, RasterTypes);

TEST(SumInBufferTest, sumInBufferCpu)
{
    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata meta(5, 4, nan);
    meta.cellSize = 5;

    MaskedRaster<float> raster(meta, std::vector<float>{
                                        2.f, nan, 4.f, 4.f,
                                        4.f, 8.f, 4.f, 9.f,
                                        2.f, 4.f, nan, 7.f,
                                        4.f, 4.f, 4.f, 8.f,
                                        3.f, nan, 4.f, -5.f});

    MaskedRaster<float> expected(meta, std::vector<float>{
                                          6.f, 14.f, 12.f, 17.f,
                                          16.f, 20.f, 25.f, 24.f,
                                          14.f, 18.f, 19.f, 24.f,
                                          13.f, 16.f, 20.f, 14.f,
                                          7.f, 11.f, 3.f, 7.f});

    auto actual = sumInBuffer(raster, 5.f);

    EXPECT_RASTER_EQ(expected, actual);
}

TEST(SumInBufferTest, maxInBufferCpu)
{
    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata meta(5, 4);
    meta.cellSize = 5;

    MaskedRaster<float> raster(meta, std::vector<float>{
                                        2.f, nan, 4.f, 4.f,
                                        4.f, 8.f, 4.f, 9.f,
                                        2.f, 4.f, nan, 7.f,
                                        4.f, 4.f, 4.f, 8.f,
                                        3.f, nan, 4.f, -5.f});

    MaskedRaster<float> expected(meta, std::vector<float>{
                                          4.f, 8.f, 4.f, 9.f,
                                          8.f, 8.f, 9.f, 9.f,
                                          4.f, 8.f, 7.f, 9.f,
                                          4.f, 4.f, 8.f, 8.f,
                                          4.f, 4.f, 4.f, 8.f});

    auto actual = maxInBuffer(raster, 5.f);

    EXPECT_RASTER_EQ(expected, actual);
}

#ifdef HAVE_OPENCL
TEST(SumInBufferTest, sumInBuffergpu)
{
    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();

    RasterMetadata meta(5, 4);
    meta.cellSize = 5;

    gpu::Raster<float> raster(meta, std::vector<float>{
                                        2.f, nan, 4.f, 4.f,
                                        4.f, 8.f, 4.f, 9.f,
                                        2.f, 4.f, nan, 7.f,
                                        4.f, 4.f, 4.f, 8.f,
                                        3.f, nan, 4.f, -5.f});

    gpu::Raster<float> expected(meta, std::vector<float>{
                                          6.f, 14.f, 12.f, 17.f,
                                          16.f, 20.f, 25.f, 24.f,
                                          14.f, 18.f, 19.f, 24.f,
                                          13.f, 16.f, 20.f, 14.f,
                                          7.f, 11.f, 3.f, 7.f});

    auto actual = sumInBuffer(raster, 5.f);

    EXPECT_EQ(expected, actual);
}
#endif
}
