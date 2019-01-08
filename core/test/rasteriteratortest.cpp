#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "gdx/test/testbase.h"

#include "gdx/denseraster.h"
#include "gdx/maskedraster.h"
#include "gdx/rasteriterator.h"
#include "gdx/rasterspan.h"
#include "gdx/sparseraster.h"
#include <vector>

namespace gdx::test {

using namespace testing;
using namespace std::string_literals;

template <class RasterType>
class RasterIteratorTest : public TestBase<RasterType>
{
public:
};

using IteratorRasterTypes = testing::Types<
    RasterValuePair<MaskedRaster, int32_t>,
    RasterValuePair<DenseRaster, int32_t>>;

TYPED_TEST_CASE(RasterIteratorTest, RasterTypes);

TYPED_TEST(RasterIteratorTest, skipNodataRaster)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    Raster ras(RasterMetadata(3, 3, 0), std::vector<T>{{0, 1, 2, 5, 0, 7, 4, 3, 0}});

    std::vector<T> expectedData = {1, 2, 5, 7, 4, 3};

    std::vector<T> result;
    std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
    EXPECT_THAT(result, Pointwise(Eq(), expectedData));
}

TYPED_TEST(RasterIteratorTest, skipNodataConstRaster)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    const Raster ras(RasterMetadata(3, 3, 0), std::vector<T>{{0, 1, 0, 0, 0, 7, 4, 3, 0}});

    std::vector<T> expectedData = {1, 7, 4, 3};

    std::vector<T> result;
    std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(expectedData));
}

TYPED_TEST(RasterIteratorTest, skipNodataConstRasterConstantBeginEnd)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    const Raster ras(RasterMetadata(3, 3, 0), std::vector<T>{{0, 1, 0, 0, 0, 7, 4, 3, 0}});

    std::vector<T> expectedData = {1, 7, 4, 3};

    std::vector<T> result;
    std::copy(value_cbegin(ras), value_cend(ras), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(expectedData));
}

TYPED_TEST(RasterIteratorTest, skipNaNNodataRaster)
{
    auto nan = std::numeric_limits<float>::quiet_NaN();

    MaskedRaster<float> ras(RasterMetadata(3, 3, nan), std::vector<float>{
                                                           nan, 0.f, 2.f,
                                                           5.f, nan, 7.f,
                                                           4.f, 9.f, nan});

    std::vector<float> expectedData = {0.f, 2.f, 5.f, 7.f, 4.f, 9.f};

    std::vector<float> result;
    std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(expectedData));
}

TYPED_TEST(RasterIteratorTest, skipNodataOnlyNodata)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    Raster ras(RasterMetadata(3, 3, 1), std::vector<T>{
                                            1, 1, 1,
                                            1, 1, 1,
                                            1, 1, 1});

    std::vector<T> result;
    std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<T>()));
}

TYPED_TEST(RasterIteratorTest, skipNodataOnlyNaNNodata)
{
    auto nan = std::numeric_limits<float>::quiet_NaN();
    MaskedRaster<float> ras(RasterMetadata(3, 3, 0), std::vector<float>{
                                                         nan, nan, nan,
                                                         nan, nan, nan,
                                                         nan, nan, nan});

    std::vector<float> result;
    std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<float>()));
}

TYPED_TEST(RasterIteratorTest, skipNodataEmptyInput)
{
    std::vector<float> data;
    MaskedRaster<float> ras;

    std::vector<float> result;
    std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<float>()));
}

TYPED_TEST(RasterIteratorTest, skipNodataEmptyInputWithNodataValue)
{
    std::vector<float> data;
    MaskedRaster<float> ras;

    std::vector<float> result;
    std::copy(value_begin(ras), value_end(ras), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<float>()));
}

TYPED_TEST(RasterIteratorTest, cellIterator)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;

    Raster ras(RasterMetadata(3, 2, 0), std::vector<T>{{0, 1,
                                            2, 0,
                                            3, 0}});

    std::vector<Cell> expectedCells = {{0, 0}, {0, 1}, {1, 0}, {1, 1}, {2, 0}, {2, 1}};

    std::vector<Cell> cells;
    std::copy(cell_begin(ras), cell_end(ras), std::back_inserter(cells));
    EXPECT_THAT(cells, ContainerEq(expectedCells));
    cells.clear();

    for (auto& cell : RasterCells(ras)) {
        cells.push_back(cell);
    }

    EXPECT_THAT(cells, ContainerEq(expectedCells));
}

TEST(RasterIteratorTest, missingValueIteratorVector)
{
    std::vector<int> ras(std::vector<int>{{0, 1, 2, 0, 3, 0}});
    RasterMetadata meta(2, 3, 0);

    auto span = make_raster_span(ras, meta);

    std::vector<int> expectedData = {1, 2, 3};
    std::vector<int> result;

    std::for_each(optional_value_begin(span), optional_value_end(span), [&](auto& value) {
        if (value) {
            result.push_back(*value);
        }
    });

    EXPECT_THAT(result, ContainerEq(expectedData));
    EXPECT_EQ(6, std::distance(optional_value_begin(span), optional_value_end(span)));
}

//TODO: currently fails because the value proxy assumes nan values for floats
//TEST(RasterIteratorTest, missingValueIteratorVectorFloat)
//{
//    std::vector<float> ras(std::vector<float>{{0, 1, 2, 0, 3, 0}});
//    RasterMetadata meta(2, 3, 0);
//
//    auto span = make_raster_span(ras, meta);
//
//    std::vector<float> expectedData = {1, 2, 3};
//    std::vector<float> result;
//
//    std::for_each(optional_value_begin(span), optional_value_end(span), [&](auto& value) {
//        if (value) {
//            result.push_back(*value);
//        }
//    });
//
//    EXPECT_THAT(result, ContainerEq(expectedData));
//    EXPECT_EQ(6, std::distance(optional_value_begin(span), optional_value_end(span)));
//}

TEST(RasterIteratorTest, missingValueIteratorDereference)
{
    std::vector<int> ras(std::vector<int>{{0, 1, 2, 0, 3, 0}});
    RasterMetadata meta(2, 3, 0);

    auto span      = make_raster_span(ras, meta);
    auto constSpan = make_raster_span(std::as_const(ras), meta);

    std::vector<int> expectedData = {1, 2, 3};
    std::vector<int> result;

    std::for_each(optional_value_begin(span), optional_value_end(span), [&](auto& value) {
        if (value) {
            result.push_back(*value);
        }
    });
    EXPECT_THAT(result, ContainerEq(expectedData));

    result.clear();
    std::for_each(optional_value_begin(constSpan), optional_value_end(constSpan), [&](auto& value) {
        if (value) {
            result.push_back(*value);
        }
    });
    EXPECT_THAT(result, ContainerEq(expectedData));
}

TEST(RasterIteratorTest, sparseRaster)
{
    std::vector<int> data(std::vector<int>{{-1, 0, 1, -1, 5, -1}});
    SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

    std::vector<int> result(data.size());
    std::copy(raster.begin(), raster.end(), result.begin());

    EXPECT_THAT(data, ContainerEq(result));
    EXPECT_EQ(data.size(), std::distance(raster.begin(), raster.end()));
}

TEST(RasterIteratorTest, sparseRasterNodataInMiddle)
{
    std::vector<int> data(std::vector<int>{{4, -1, -1, -1, -1, 5}});
    SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

    std::vector<int> result(data.size());
    std::copy(raster.begin(), raster.end(), result.begin());
    EXPECT_THAT(data, ContainerEq(result));
    EXPECT_EQ(data.size(), std::distance(raster.begin(), raster.end()));
}

TEST(RasterIteratorTest, sparseRasterOnlyNodata)
{
    std::vector<int> data(std::vector<int>{{-1, -1, -1, -1, -1, -1}});

    SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

    std::vector<int> result(data.size());
    std::copy(raster.begin(), raster.end(), result.begin());

    EXPECT_THAT(data, ContainerEq(result));
}

TEST(RasterIteratorTest, sparseRasterOnlyData)
{
    std::vector<int> data(std::vector<int>{{1, 2, 3, 4, 5, 6}});

    SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

    std::vector<int> result(data.size());
    std::copy(raster.begin(), raster.end(), result.begin());

    EXPECT_THAT(result, ContainerEq(data));
}

TEST(RasterIteratorTest, onlyDataValues)
{
    std::vector<int> data(std::vector<int>{{4, -1, -1, -1, -1, 5}});
    SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

    std::vector<int> result;
    std::copy(value_begin(raster), value_end(raster), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<int>({4, 5})));
}

TEST(RasterIteratorTest, onlyDataValuesInMiddle)
{
    std::vector<int> data(std::vector<int>{{-1, 1, 2, 3, 4, -1}});
    SparseRaster<int> raster(RasterMetadata(3, 2, -1), data);

    std::vector<int> result;
    std::copy(value_begin(raster), value_end(raster), std::back_inserter(result));
    EXPECT_THAT(result, ContainerEq(std::vector<int>({1, 2, 3, 4})));
}

}
