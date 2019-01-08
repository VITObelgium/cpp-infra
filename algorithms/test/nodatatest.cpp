#include "gdx/algo/nodata.h"
#include "gdx/algo/logical.h"
#include "gdx/test/testbase.h"

#include <numeric>
#include <random>

namespace gdx::test {

template <class RasterType>
class NodataTest : public TestBase<RasterType>
{
};

TYPED_TEST_CASE(NodataTest, RasterTypes);

TYPED_TEST(NodataTest, is_nodata)
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

    Raster raster1(RasterMetadata(5, 4), convertTo<T>(v));
    Raster raster2(RasterMetadata(5, 4, -999), convertTo<T>(v));

    MaskedRaster<uint8_t> expected1(RasterMetadata(5, 4), std::vector<uint8_t>{
                                                              0, 0, 0, 0,
                                                              0, 0, 0, 0,
                                                              0, 0, 0, 0,
                                                              0, 0, 0, 0,
                                                              0, 0, 0, 0});

    MaskedRaster<uint8_t> expected2(RasterMetadata(5, 4), std::vector<uint8_t>{
                                                              1, 1, 0, 0,
                                                              0, 0, 0, 0,
                                                              0, 0, 1, 0,
                                                              0, 0, 0, 0,
                                                              0, 1, 0, 1});

    MaskedRaster<uint8_t> output(RasterMetadata(5, 4));
    is_nodata(raster1, output);
    EXPECT_RASTER_EQ(expected1, output);

    is_nodata(raster2, output);
    EXPECT_RASTER_EQ(expected2, output);

    is_data(raster1, output);
    EXPECT_RASTER_EQ(expected1, !output);

    is_data(raster2, output);
    EXPECT_RASTER_EQ(expected2, !output);
}

TYPED_TEST(NodataTest, is_nodataAllis_nodata)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    RasterMetadata meta(5, 4);
    meta.nodata = -999;

    Raster raster(meta, std::vector<T>(meta.rows * meta.cols, static_cast<T>(*meta.nodata)));
    MaskedRaster<uint8_t> output(5, 4);

    is_nodata(raster, output);
    EXPECT_TRUE(all_of(output));
}

TYPED_TEST(NodataTest, replaceNodata)
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

    Raster raster(RasterMetadata(5, 4, -999), convertTo<T>(v));

    Raster expected(RasterMetadata(5, 4), convertTo<T>(std::vector<double>{
                                              44.0, 44.0, 4.0, 4.0,
                                              4.0, 8.0, 4.0, 9.0,
                                              2.0, 4.0, 44.0, 7.0,
                                              4.0, 4.0, -5.0, 8.0,
                                              3.0, 44.0, 4.0, 44.0}));

    replace_nodata_in_place(raster, T(44));
    EXPECT_RASTER_EQ(expected, raster);

    MaskedRaster<uint8_t> isDataOutput(5, 4);
    is_data(raster, isDataOutput);
    EXPECT_TRUE(all_of(isDataOutput));
}

TYPED_TEST(NodataTest, turnValueIntoNodata)
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

    Raster raster(RasterMetadata(5, 4, -999), convertTo<T>(v));

    Raster expected(RasterMetadata(5, 4, -999), convertTo<T>(std::vector<double>{
                                                    -999.0, -999.0, -999.0, -999.0,
                                                    -999.0, 8.0, -999.0, 9.0,
                                                    2.0, -999.0, -999.0, 7.0,
                                                    -999.0, -999.0, -5.0, 8.0,
                                                    3.0, -999.0, -999.0, -999.0}));

    turn_value_into_nodata(raster, T(4.0));
    EXPECT_RASTER_EQ(expected, raster);
}
}
