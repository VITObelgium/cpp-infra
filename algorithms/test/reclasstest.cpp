#include "gdx/algo/reclass.h"
#include "gdx/test/testbase.h"

#include <fstream>

namespace gdx::test {

class ReclassTest : public Test
{
};

TEST(ReclassTest, reclass)
{
    const MaskedRaster<int32_t> ras(4, 5, std::vector<int32_t>{1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 1, 1, 1, 1});

    const MaskedRaster<int32_t> expected(ras.metadata(), std::vector<int32_t>{
                                                            10, 10, 20, 20,
                                                            20, 20, 30, 30,
                                                            30, 30, 40, 40,
                                                            40, 40, 50, 50,
                                                            10, 10, 10, 10});

    const std::vector<std::vector<double>> map({{0, 0},
        {1, 10},
        {2, 20},
        {3, 30},
        {4, 40},
        {5, 50}});

    EXPECT_RASTER_EQ(expected, reclass(map, ras));
}

TEST(ReclassTest, reclassNodata)
{
    static const std::string mapping = R"(1	10
2	20
3	30
4	nan
nan	0
)";

    {
        std::ofstream file("reclass_nodata.tab");
        ASSERT_TRUE(file.is_open());
        file << mapping;
    }

    RasterMetadata meta(5, 4);
    meta.nodata = -1;

    const MaskedRaster<int32_t> ras(meta, std::vector<int32_t>{
                                             1, 1, 2, 2,
                                             2, 2, 3, -1,
                                             3, 3, -1, 4,
                                             4, -1, 4, 4,
                                             -1, 1, 1, 1});

    const MaskedRaster<int32_t> expected(ras.metadata(), std::vector<int32_t>{
                                                            10, 10, 20, 20,
                                                            20, 20, 30, 0,
                                                            30, 30, 0, -1,
                                                            -1, 0, -1, -1,
                                                            0, 10, 10, 10});

    auto result = reclass("reclass_nodata.tab", ras);
    EXPECT_RASTER_EQ(expected, result);
}

TEST(ReclassTest, reclassNodataInvalidMappingFile)
{
    // space instead of tab between the two nan values on the last line
    static const std::string mapping = R"(1	10
nan nan
)";

    {
        std::ofstream file("reclass_nodata.tab");
        ASSERT_TRUE(file.is_open());
        file << mapping;
    }

    RasterMetadata meta(5, 4);
    meta.nodata = -1;

    const MaskedRaster<int32_t> ras(meta, std::vector<int32_t>{
                                             1, 1, 2, 2,
                                             2, 2, 3, -1,
                                             3, 3, -1, 4,
                                             4, -1, 4, 4,
                                             -1, 1, 1, 1});

    EXPECT_THROW(reclass("reclass_nodata.tab", ras), RuntimeError);
}

TEST(ReclassTest, reclassFloatResult)
{
    static const std::string mapping = R"(1	1
2	0.8
3	0.6
4	0.4
5	0.15
)";

    {
        std::ofstream file("reclass_floatresult.tab");
        ASSERT_TRUE(file.is_open());
        file << mapping;
    }

    RasterMetadata meta(5, 4);
    meta.nodata = -1;

    const MaskedRaster<float> ras(meta, std::vector<float>{
                                           1, 1, 2, 2,
                                           2, 2, 3, -1,
                                           3, 3, -1, 4,
                                           4, -1, 4, 4,
                                           -1, 1, 1, 1});

    auto nan = std::numeric_limits<float>::quiet_NaN();

    const MaskedRaster<float> expected(ras.metadata(), std::vector<float>{
                                                          1.f, 1.f, 0.8f, 0.8f,
                                                          0.8f, 0.8f, 0.6f, nan,
                                                          0.6f, 0.6f, nan, 0.4f,
                                                          0.4f, nan, 0.4f, 0.4f,
                                                          nan, 1.f, 1.f, 1.f});

    auto result = reclass("reclass_floatresult.tab", ras);
    EXPECT_RASTER_NEAR(expected, result);
}

TEST(ReclassTest, reclassInPlace)
{
    MaskedRaster<int32_t> ras(4, 5, std::vector<int32_t>{1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 1, 1, 1, 1});

    const MaskedRaster<int32_t> expected(ras.metadata(), std::vector<int32_t>{
                                                            10, 10, 20, 20,
                                                            20, 20, 30, 30,
                                                            30, 30, 40, 40,
                                                            40, 40, 50, 50,
                                                            10, 10, 10, 10});

    const std::vector<std::vector<double>> map({{0, 0},
        {1, 10},
        {2, 20},
        {3, 30},
        {4, 40},
        {5, 50}});
    reclassInPlace(map, ras);
    EXPECT_RASTER_EQ(expected, ras);
}

TEST(ReclassTest, reclassMultipleInputsNodata)
{
    static const std::string mapping = R"(1	1	0
2	3	nan
3	2	1
1	nan	10
nan	1	20
nan	nan	30
)";

    {
        std::ofstream file("reclass_nodata.tab");
        ASSERT_TRUE(file.is_open());
        file << mapping;
    }

    RasterMetadata meta(2, 3);
    meta.nodata = -1;

    const MaskedRaster<int32_t> ras1(meta, std::vector<int32_t>{
                                              1, 1, 2,
                                              -1, -1, 3});

    const MaskedRaster<int32_t> ras2(meta, std::vector<int32_t>{
                                              1, -1, 3,
                                              1, -1, 2});

    const MaskedRaster<int32_t> expected(meta, std::vector<int32_t>{
                                                  0, 10, -1,
                                                  20, 30, 1});

    auto result = reclass("reclass_nodata.tab", ras1, ras2);
    EXPECT_RASTER_EQ(expected, result);
}

TEST(ReclassTest, reclassInvalidMappingNoNodata)
{
    const std::vector<std::vector<double>> map({{0, 0},
        {1, 10},
        {2, 20},
        {3, 30},
        {4, 40}});

    const MaskedRaster<int32_t> ras(4, 5, std::vector<int32_t>{1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 1, 1, 1, 1});

    EXPECT_THROW(reclass(map, ras), RuntimeError) << reclass(map, ras).eigen_const_data();
}

TEST(ReclassTest, reclassInvalidMappingHasNodata)
{
    const std::vector<std::vector<double>> map({{0, 0},
        {1, 10},
        {2, 20},
        {3, 30},
        {4, 40}});

    RasterMetadata meta(4, 5);
    meta.nodata = -1;
    const MaskedRaster<int32_t> ras(meta, std::vector<int32_t>{
                                             1, 1, 2, 2,
                                             2, 2, 3, 3,
                                             3, 3, 4, 4,
                                             4, 4, 5, 5,
                                             1, 1, 1, 1});

    const MaskedRaster<int32_t> expected(ras.metadata(), std::vector<int32_t>{
                                                            10, 10, 20, 20,
                                                            20, 20, 30, 30,
                                                            30, 30, 40, 40,
                                                            40, 40, -1, -1,
                                                            10, 10, 10, 10});

    auto result = reclass(map, ras);
    EXPECT_RASTER_EQ(expected, result);
}

TEST(ReclassTest, nreclass)
{
    static const std::string mapping = R"(-3	-1	10
-1	0	11
0	1	12
1	2	13
2	3	14
)";

    {
        std::ofstream file("nreclass.tab");
        ASSERT_TRUE(file.is_open());
        file << mapping;
    }

    RasterMetadata meta(5, 10);
    meta.nodata = -9999;

    const MaskedRaster<float> ras(meta, std::vector<float>{
                                           -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2, 2.5,
                                           -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2, 2.5,
                                           -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2, 2.5,
                                           -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2, 2.5,
                                           -2, -1.5, -1, -0.5, 0, 0.5, 1, 1.5, 2, 2.5});

    const MaskedRaster<float> expected(meta, std::vector<float>{
                                                10, 10, 10, 11, 11, 12, 12, 13, 13, 14,
                                                10, 10, 10, 11, 11, 12, 12, 13, 13, 14,
                                                10, 10, 10, 11, 11, 12, 12, 13, 13, 14,
                                                10, 10, 10, 11, 11, 12, 12, 13, 13, 14,
                                                10, 10, 10, 11, 11, 12, 12, 13, 13, 14});

    auto result = nreclass("nreclass.tab", ras);
    EXPECT_RASTER_EQ(expected, result);
}

TEST(ReclassTest, reclassiNodata)
{
    static const std::string mapping = R"(1	10	100
2	20	200
3	30	300
4	nan	nan
nan	0	nan
)";

    {
        std::ofstream file("reclassi_nodata.tab");
        ASSERT_TRUE(file.is_open());
        file << mapping;
    }

    RasterMetadata meta(5, 4);
    meta.nodata = -1;

    const MaskedRaster<int32_t> ras(meta, std::vector<int32_t>{
                                             1, 1, 2, 2,
                                             2, 2, 3, -1,
                                             3, 3, -1, 4,
                                             4, -1, 4, 4,
                                             -1, 1, 1, 1});

    const MaskedRaster<int32_t> expected1(ras.metadata(), std::vector<int32_t>{
                                                             10, 10, 20, 20,
                                                             20, 20, 30, 0,
                                                             30, 30, 0, -1,
                                                             -1, 0, -1, -1,
                                                             0, 10, 10, 10});

    const MaskedRaster<int32_t> expected2(ras.metadata(), std::vector<int32_t>{
                                                             100, 100, 200, 200,
                                                             200, 200, 300, -1,
                                                             300, 300, -1, -1,
                                                             -1, -1, -1, -1,
                                                             -1, 100, 100, 100});

    auto result = reclassi("reclassi_nodata.tab", ras, 1);
    EXPECT_RASTER_EQ(expected1, result);

    result = reclassi("reclassi_nodata.tab", ras, 2);
    EXPECT_RASTER_EQ(expected2, result);
}

#ifdef HAVE_OPENCL
TEST(ReclassTest, reclassInPlaceGpu)
{
    gpu::Raster<int32_t> ras(4, 5, std::vector<int32_t>{1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 1, 1, 1, 1});

    const gpu::Raster<int32_t> expected(ras.metadata(), std::vector<int32_t>{
                                                            10, 10, 20, 20,
                                                            20, 20, 30, 30,
                                                            30, 30, 40, 40,
                                                            40, 40, 50, 50,
                                                            10, 10, 10, 10});

    const std::vector<double> map{{0, 10, 20, 30, 40, 50}};
    reclassInPlace<int32_t>(ras, map);
    EXPECT_EQ(expected, ras);
}

TEST(ReclassTest, reclassGpu)
{
    const gpu::Raster<int32_t> ras(4, 5, std::vector<int32_t>{1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 1, 1, 1, 1});

    const gpu::Raster<int32_t> expected(ras.metadata(), std::vector<int32_t>{
                                                            10, 10, 20, 20,
                                                            20, 20, 30, 30,
                                                            30, 30, 40, 40,
                                                            40, 40, 50, 50,
                                                            10, 10, 10, 10});

    const std::vector<double> map{{0, 10, 20, 30, 40, 50}};
    EXPECT_EQ(expected, reclass<int32_t>(ras, map));
}
#endif

//TEST(ReclassTest, reclassNoData)
//{
//    constexpr auto nan = std::numeric_limits<float>::quiet_NaN();
//
//    const std::vector<float> v = {
//        2.f, nan, 4.f, 4.f,
//        4.f, 8.f, 4.f, 9.f,
//        2.f, 4.f, nan, 7.f,
//        4.f, 4.f, 4.f, 8.f,
//        3.f, nan, 4.f,-5.f
//    };
//
//    gpu::Raster<float> raster(5, 4, std::move(v));
//    EXPECT_EQ(-5.f, minimum(raster));
//}
}
