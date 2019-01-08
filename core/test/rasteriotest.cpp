#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "gdx/test/comparisons.h"
#include "testconfig.h"

#include "gdx/rasterspanio.h"

#include "gdx/algo/cast.h"
#include "gdx/denserasterio.h"
#include "gdx/maskedrasterio.h"
#include "gdx/sparserasterio.h"
#include "infra/string.h"

#include "gdx/test/testbase.h"

#include <fmt/format.h>
#include <fstream>
#include <iterator>

namespace gdx::test {

using namespace inf;
using namespace testing;
using namespace std::string_literals;

template <class RasterType>
class TypedRasterIOTest : public TestBase<RasterType>
{
};

template <class RasterType>
class TypedDenseRasterIntIOTest : public TestBase<RasterType>
{
};

template <class RasterType>
class TypedDenseRasterFloatIOTest : public TestBase<RasterType>
{
};

template <class RasterType>
class ContiguousRasterIOTest : public TestBase<RasterType>
{
};

using RasterIOTypes = testing::Types<MaskedRaster<float>, DenseRaster<float>, std::vector<float>>;
TYPED_TEST_CASE(TypedRasterIOTest, RasterIOTypes);

using ContiguousRasterTypes = testing::Types<MaskedRaster<float>, DenseRaster<float>>;
TYPED_TEST_CASE(ContiguousRasterIOTest, ContiguousRasterTypes);

using DenseRasterIntIOTypes = testing::Types<MaskedRaster<int32_t>, DenseRaster<int32_t>>;
TYPED_TEST_CASE(TypedDenseRasterIntIOTest, DenseRasterIntIOTypes);

using DenseRasterFloatIOTypes = testing::Types<MaskedRaster<float>, DenseRaster<float>>;
TYPED_TEST_CASE(TypedDenseRasterFloatIOTest, DenseRasterFloatIOTypes);

TYPED_TEST(TypedRasterIOTest, read_rasterAsFloat)
{
    TypeParam raster;
    auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", raster);

    RasterMetadata expectedMeta(3, 5);
    expectedMeta.cellSize = 4.0;
    expectedMeta.xll      = 1.0;
    expectedMeta.yll      = -10.0;
    compareMetaData(expectedMeta, meta);

    std::vector<float> expectedData = {
        0.f, 1.f, 2.f, 3.f, 4.f,
        5.f, 6.f, 7.f, 8.f, 9.f,
        4.f, 3.f, 2.f, 1.f, 0.f};

    EXPECT_THAT(raster, Pointwise(FloatEqNan(), expectedData));
}

TEST(RasterIOTest, readMetadata)
{
    auto meta = io::read_metadata(TEST_DATA_DIR "/testraster-nodata.asc");

    EXPECT_EQ(3, meta.rows);
    EXPECT_EQ(5, meta.cols);
    EXPECT_TRUE(meta.nodata.has_value());
    EXPECT_DOUBLE_EQ(-1.0, *meta.nodata);
    EXPECT_DOUBLE_EQ(4.0, meta.cellSize);
    EXPECT_DOUBLE_EQ(1.0, meta.xll);
    EXPECT_DOUBLE_EQ(-10.0, meta.yll);
}

TEST(RasterIOTest, readMetadataWithProjection)
{
    auto meta = io::read_metadata(TEST_DATA_DIR "/../../../test/mapdata/landusebyte.tif");
    EXPECT_EQ("EPSG:31370", meta.projection_frienly_name());
}

TYPED_TEST(TypedDenseRasterIntIOTest, read_rasterAsInt)
{
    TypeParam ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", ras);

    EXPECT_EQ(3, meta.rows);
    EXPECT_EQ(5, meta.cols);
    EXPECT_FALSE(meta.nodata.has_value());
    EXPECT_DOUBLE_EQ(4.0, meta.cellSize);
    EXPECT_DOUBLE_EQ(1.0, meta.xll);
    EXPECT_DOUBLE_EQ(-10.0, meta.yll);

    std::vector<int32_t> expectedData = {
        0, 1, 2, 3, 4,
        5, 6, 7, 8, 9,
        4, 3, 2, 1, 0};

    EXPECT_THAT(ras, Pointwise(Eq(), expectedData));
}

// Testraster
// llx 1
// lly -10
// 0, 1, 2, 3, 4
// 5, 6, 7, 8, 9
// 4, 3, 2, 1, 0

TEST(RasterIOTest, read_rasterAsIntWithExtentWhichIsContained)
{
    RasterMetadata extent(2, 2);
    extent.xll      = 9.0;
    extent.yll      = -10.0;
    extent.cellSize = 4.0;

    auto ras = read_masked_raster<int32_t>(TEST_DATA_DIR "/testraster.asc", extent);

    EXPECT_EQ(2, ras.metadata().rows);
    EXPECT_EQ(2, ras.metadata().cols);
    EXPECT_FALSE(ras.metadata().nodata.has_value());
    EXPECT_DOUBLE_EQ(4.0, ras.metadata().cellSize);
    EXPECT_DOUBLE_EQ(9.0, ras.metadata().xll);
    EXPECT_DOUBLE_EQ(-10.0, ras.metadata().yll);

    std::vector<int32_t> expectedData = {
        7, 8,
        2, 1};

    EXPECT_THAT(ras, Pointwise(Eq(), expectedData));
}

TEST(RasterIOTest, read_rasterAsIntWithExtentWhichIsNotContainedBottomLeft)
{
    RasterMetadata extent(3, 4);
    extent.xll      = -3.0;
    extent.yll      = -14.0;
    extent.cellSize = 4.0;

    MaskedRaster<int32_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", extent, ras);
    auto nod  = std::numeric_limits<int32_t>::max();

    EXPECT_EQ(3, meta.rows);
    EXPECT_EQ(4, meta.cols);
    EXPECT_TRUE(meta.nodata.has_value());
    EXPECT_DOUBLE_EQ(4.0, meta.cellSize);
    EXPECT_DOUBLE_EQ(-3.0, meta.xll);
    EXPECT_DOUBLE_EQ(-14.0, meta.yll);

    MaskedRaster<int32_t> expected(RasterMetadata(3, 4, double(nod)), std::vector<int32_t>{
                                                                          nod, 5, 6, 7,
                                                                          nod, 4, 3, 2,
                                                                          nod, nod, nod, nod});

    EXPECT_RASTER_EQ(expected, ras);
}

TEST(RasterIOTest, read_rasterAsIntWithExtentWhichIsNotContainedTopLeft)
{
    RasterMetadata extent(3, 4);
    extent.xll      = -3.0;
    extent.yll      = -6.0;
    extent.cellSize = 4.0;

    MaskedRaster<int32_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", extent, ras);

    EXPECT_EQ(3, meta.rows);
    EXPECT_EQ(4, meta.cols);
    EXPECT_TRUE(meta.nodata.has_value());
    EXPECT_DOUBLE_EQ(4.0, meta.cellSize);
    EXPECT_DOUBLE_EQ(-3.0, meta.xll);
    EXPECT_DOUBLE_EQ(-6.0, meta.yll);

    MaskedRaster<int32_t> expected(RasterMetadata(3, 4, -1), std::vector<int32_t>{
                                                                 -1, -1, -1, -1,
                                                                 -1, 0, 1, 2,
                                                                 -1, 5, 6, 7});

    EXPECT_TRUE(expected.tolerant_data_equal_to(ras));
}

TEST(RasterIOTest, read_rasterAsIntWithExtentWhichIsNotContainedTopRight)
{
    RasterMetadata extent(3, 4);
    extent.xll      = 13.0;
    extent.yll      = -6.0;
    extent.cellSize = 4.0;

    MaskedRaster<int32_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", extent, ras);

    EXPECT_EQ(3, meta.rows);
    EXPECT_EQ(4, meta.cols);
    EXPECT_TRUE(meta.nodata.has_value());
    EXPECT_DOUBLE_EQ(4.0, meta.cellSize);
    EXPECT_DOUBLE_EQ(13.0, meta.xll);
    EXPECT_DOUBLE_EQ(-6.0, meta.yll);

    MaskedRaster<int32_t> expected(RasterMetadata(3, 4, -1), std::vector<int32_t>{
                                                                 -1, -1, -1, -1,
                                                                 3, 4, -1, -1,
                                                                 8, 9, -1, -1});

    EXPECT_TRUE(expected.tolerant_data_equal_to(ras));
}

TEST(RasterIOTest, read_rasterAsIntWithExtentWhichIsNotContainedBottomRight)
{
    RasterMetadata extent(3, 4);
    extent.xll      = 9.0;
    extent.yll      = -18.0;
    extent.cellSize = 4.0;

    MaskedRaster<int32_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster.asc", extent, ras);

    EXPECT_EQ(3, meta.rows);
    EXPECT_EQ(4, meta.cols);
    EXPECT_TRUE(meta.nodata.has_value());
    EXPECT_DOUBLE_EQ(4.0, meta.cellSize);
    EXPECT_DOUBLE_EQ(9.0, meta.xll);
    EXPECT_DOUBLE_EQ(-18.0, meta.yll);

    MaskedRaster<int32_t> expected(RasterMetadata(3, 4, -1), std::vector<int32_t>{
                                                                 2, 1, 0, -1,
                                                                 -1, -1, -1, -1,
                                                                 -1, -1, -1, -1});

    EXPECT_TRUE(expected.tolerant_data_equal_to(ras));
}

TEST(RasterIOTest, readFloatRasterAsByte)
{
    MaskedRaster<uint8_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/floatraster-negative-nodata.tif", ras);

    EXPECT_EQ(40, meta.rows);
    EXPECT_EQ(49, meta.cols);
    EXPECT_EQ(255, meta.nodata);
    EXPECT_DOUBLE_EQ(100.0, meta.cellSize);
    EXPECT_DOUBLE_EQ(209400, meta.xll);
    EXPECT_DOUBLE_EQ(217000, meta.yll);

    // The upper right corner contains nodata
    EXPECT_TRUE(ras.is_nodata(0, 48));
    //EXPECT_EQ(255, ras(0, 48));
}

TEST(RasterIOTest, sameCastAndReadBehaviour)
{
    // Casting while reading should have te same behavior as casting after reading

    MaskedRaster<uint8_t> ras1;
    MaskedRaster<float> ras2;

    read_raster(TEST_DATA_DIR "/floatraster-negative-nodata.tif", ras1);
    read_raster(TEST_DATA_DIR "/floatraster-negative-nodata.tif", ras2);
    auto ras2Uint = raster_cast<uint8_t>(ras2);

    EXPECT_EQ(ras1, ras2Uint);
    EXPECT_EQ(ras1.metadata(), ras2Uint.metadata());
}

TEST(RasterIOTest, castToFloatAndBack)
{
    std::vector<uint8_t> data = {
        255, 1, 2,
        3, 4, 5,
        0, 0, 255};

    RasterMetadata meta(3, 3);
    meta.nodata = 255;
    MaskedRaster<uint8_t> ras(meta, data);

    auto floatRaster = raster_cast<float>(ras);
    auto result      = raster_cast<uint8_t>(floatRaster);
    EXPECT_EQ(ras, result);
    EXPECT_EQ(ras.metadata(), result.metadata());
}

TEST(RasterIOTest, saveRasterAsPng)
{
    MaskedRaster<uint8_t> ras;
    read_raster(TEST_DATA_DIR "/testraster.asc", ras);
    EXPECT_NO_THROW(write_raster<uint8_t>(ras, "test.png"));
}

TEST(RasterIOTest, saveRasterAsGif)
{
    if (!gdal::RasterDriver::isSupported(gdal::RasterType::Gif)) {
        return;
    }

    MaskedRaster<uint8_t> ras;
    read_raster(TEST_DATA_DIR "/testraster.asc", ras);
    write_raster<uint8_t>(ras, "test.gif");
}

TEST(RasterIOTest, saveRasterAsTif)
{
    MaskedRaster<uint8_t> ras;
    read_raster(TEST_DATA_DIR "/testraster.asc", ras);
    write_raster<uint8_t>(ras, "test.tif");
}

template <typename T>
void testRaster(std::string_view filename)
{
    MaskedRaster<int32_t> referenceRaster;
    referenceRaster.set_metadata(read_raster(fs::path(TEST_DATA_DIR) / std::string(filename), referenceRaster));
    write_raster(referenceRaster, "raster.asc");

    MaskedRaster<int32_t> writtenRaster;
    auto meta = read_raster<>("raster.asc", writtenRaster);

    compareMetaData(referenceRaster.metadata(), meta);
    EXPECT_THAT(referenceRaster, ContainerEq(writtenRaster));
}

TEST(RasterIOTest, write_rasterAsInt)
{
    testRaster<int32_t>("testraster.asc");
}

TEST(RasterIOTest, write_raster_as_float)
{
    testRaster<float>("testraster.asc");
}

TEST(RasterIOTest, read_rasterWithNodata)
{
    MaskedRaster<float> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster-nodata.asc", ras);

    EXPECT_EQ(3, meta.rows);
    EXPECT_EQ(5, meta.cols);
    EXPECT_DOUBLE_EQ(-1.0, meta.nodata.value());
    EXPECT_DOUBLE_EQ(4.0, meta.cellSize);
    EXPECT_DOUBLE_EQ(1.0, meta.xll);
    EXPECT_DOUBLE_EQ(-10.0, meta.yll);

    MaskedRaster<float> expected(ras.metadata(), std::vector<float>{
                                                     0.f, 1.f, 2.f, 3.f, 4.f,
                                                     5.f, 6.f, 7.f, 8.f, 9.f,
                                                     4.f, 3.f, 2.f, -1.f, 0.f});

    EXPECT_TRUE(ras.is_nodata(2, 3));
    EXPECT_TRUE(expected.tolerant_equal_to(ras));
}

TEST(RasterIOTest, read_rasterWithNodataAsInt)
{
    MaskedRaster<int32_t> ras;
    auto meta = read_raster(TEST_DATA_DIR "/testraster-nodata.asc", ras);

    EXPECT_EQ(3, meta.rows);
    EXPECT_EQ(5, meta.cols);
    EXPECT_DOUBLE_EQ(-1.0, meta.nodata.value());
    EXPECT_DOUBLE_EQ(4.0, meta.cellSize);
    EXPECT_DOUBLE_EQ(1.0, meta.xll);
    EXPECT_DOUBLE_EQ(-10.0, meta.yll);

    std::vector<int32_t> expectedData = {
        0, 1, 2, 3, 4,
        5, 6, 7, 8, 9,
        4, 3, 2, -1, 0};

    EXPECT_TRUE(ras.is_nodata(2, 3));
    EXPECT_THAT(ras, Pointwise(Eq(), expectedData));
}

TEST(RasterIOTest, read_rasterFromMemory)
{
    static const std::string raster =
        "ncols        5\n"
        "nrows        3\n"
        "xllcorner    1.000000000000\n"
        "yllcorner    -10.000000000000\n"
        "cellsize     4.000000000000\n"
        "NODATA_value  -1\n"
        "0 1 2 3 4\n"
        "5 6 7 8 9\n"
        "4 3 2 -1 0\n";

    io::gdal::MemoryFile memFile("/vsimem/ras.asc", gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(raster.data()), raster.size()));
    MaskedRaster<int16_t> ras;
    auto meta = read_raster(memFile.path(), ras);
    EXPECT_EQ(3, meta.rows);
    EXPECT_EQ(5, meta.cols);
}

TEST(RasterIOTest, write_rasterWithNodataFloat)
{
    MaskedRaster<float> referenceRaster;
    referenceRaster.set_metadata(read_raster(TEST_DATA_DIR "/testraster-nodata.asc", referenceRaster));
    write_raster(referenceRaster, "raster.asc");

    std::vector<std::string> expected({
        "ncols        5"s,
        "nrows        3"s,
        "xllcorner    1.000000000000"s,
        "yllcorner    -10.000000000000"s,
        "cellsize     4.000000000000"s,
        "NODATA_value  -1"s,
        "0.0 1 2 3 4"s,
        "5 6 7 8 9"s,
        "4 3 2 -1 0"s,
    });

    std::ifstream str("raster.asc");
    ASSERT_TRUE(str.is_open());
    std::vector<std::string> actual;
    std::string line;
    while (std::getline(str, line)) {
        actual.push_back(str::trim(line));
    }

    EXPECT_THAT(actual, ContainerEq(expected));
}

TYPED_TEST(TypedDenseRasterFloatIOTest, warp_raster)
{
    TypeParam referenceRaster;
    read_raster(TEST_DATA_DIR "/../../../test/mapdata/landusebyte.tif", referenceRaster);
    auto result = warp_raster(referenceRaster, 3857);

    EXPECT_TRUE(referenceRaster.is_nodata(0, 0));
    EXPECT_TRUE(result.is_nodata(0, 0));

    EXPECT_FALSE(referenceRaster.is_nodata(500, 1685));
    EXPECT_FALSE(result.is_nodata(500, 1685));

    EXPECT_EQ(3857, result.metadata().projection_epsg().value());
    EXPECT_EQ(4326, result.metadata().projection_geo_epsg().value());
}

TYPED_TEST(ContiguousRasterIOTest, resampleToHigherCellsize)
{
    using T = typename TypeParam::value_type;

    // clang-format off
    RasterMetadata meta(3, 2, 0);
    meta.cellSize = 100;
    meta.set_projection_from_epsg(31370);

    TypeParam ras(meta, std::vector<T>({
        1, 2,
        3, 4,
        5, 0,
        }));

    meta.rows = 6;
    meta.cols = 4;
    meta.cellSize = 50.0;
    const TypeParam expected(meta, std::vector<T>({
        1, 1, 2, 2,
        1, 1, 2, 2,
        3, 3, 4, 4,
        3, 3, 4, 4,
        5, 5, 0, 0,
        5, 5, 0, 0,
        }));
    // clang-format on

    EXPECT_RASTER_EQ(expected, resample_raster(ras, meta, gdal::ResampleAlgorithm::NearestNeighbour));
}

TYPED_TEST(ContiguousRasterIOTest, resampleToLowerCellsizeAverage)
{
    using T = typename TypeParam::value_type;

    // clang-format off
    RasterMetadata meta(6, 4);
    meta.cellSize = 50.0;
    meta.set_projection_from_epsg(31370);

    TypeParam ras(meta, std::vector<T>({
        1, 2, 2, 3,
        3, 6, 4, 3,
        4, 5, 5, 6,
        6, 9, 7, 10,
        0, 0, 1, 1,
        0, 0, 1, 1,
        }));

    meta.rows = 3;
    meta.cols = 2;
    meta.cellSize = 100.0;
    const TypeParam expectedNearest(meta, std::vector<T>({
        6, 3,
        9, 10,
        0, 1,
        }));
    const TypeParam expectedAverage(meta, std::vector<T>({
        3, 3,
        6, 7,
        0, 1,
        }));
    const TypeParam expectedMin(meta, std::vector<T>({
        1, 2,
        4, 5,
        0, 1,
        }));
    // clang-format on

    EXPECT_RASTER_EQ(expectedNearest, resample_raster(ras, meta, gdal::ResampleAlgorithm::NearestNeighbour));
    EXPECT_RASTER_EQ(expectedAverage, resample_raster(ras, meta, gdal::ResampleAlgorithm::Average));
    EXPECT_RASTER_EQ(expectedMin, resample_raster(ras, meta, gdal::ResampleAlgorithm::Minimum));
}

}
