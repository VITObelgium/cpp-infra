#include "gdx/algo/logical.h"
#include "gdx/test/testbase.h"

#include <numeric>
#include <random>

namespace gdx::test {

template <class RasterType>
class LogicalTest : public TestBase<RasterType>
{
public:
    LogicalTest()
    {
        using Raster = typename RasterType::raster;

        nodata      = 100.0;
        nodataOrNan = nodata;
        if constexpr (Raster::raster_type_has_nan) {
            nodataOrNan = std::numeric_limits<double>::quiet_NaN();
        }

        meta        = RasterMetadata(3, 3);
        meta.nodata = nodataOrNan;
    }

    RasterMetadata meta;
    double nodata;
    double nodataOrNan;
};

TYPED_TEST_CASE(LogicalTest, RasterTypes);

TYPED_TEST(LogicalTest, all)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    EXPECT_FALSE(all_of(Raster(2, 2, convertTo<T>(std::vector<double>{1.0, 1.0, 1.0, 0.0}))));
    EXPECT_TRUE(all_of(Raster(2, 2, convertTo<T>(std::vector<double>{1.0, 1.0, 1.0, 1.0}))));

    RasterMetadata meta(2, 2);
    meta.nodata = 100;

    EXPECT_TRUE(all_of(Raster(meta, convertTo<T>(std::vector<double>{
                                       100.0, 100.0,
                                       100.0, 100.0}))));
}

TYPED_TEST(LogicalTest, any)
{
    using T      = typename TypeParam::value_type;
    using Raster = typename TypeParam::raster;
    if (!typeSupported<T>()) return;

    EXPECT_FALSE(any_of(Raster(2, 2, convertTo<T>(std::vector<double>{0.0, 0.0, 0.0, 0.0}))));
    EXPECT_TRUE(any_of(Raster(2, 2, convertTo<T>(std::vector<double>{0.0, 0.0, 0.0, 1.0}))));
    EXPECT_TRUE(any_of(Raster(2, 2, convertTo<T>(std::vector<double>{1.0, 1.0, 1.0, 1.0}))));

    RasterMetadata meta(2, 2);
    meta.nodata = 100;

    EXPECT_FALSE(any_of(Raster(meta, convertTo<T>(std::vector<double>{
                                        100.0, 0.0,
                                        100.0, 0.0}))));
}
}
