#include "testconfig.h"

#include "gdx/denseraster.h"
#include "gdx/maskedraster.h"
#include "gdx/sparseraster.h"
#include "gdx/test/platformsupport.h"
#include "gdx/test/printsupport.h"
#include "gdx/test/rasterasserts.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <limits>

namespace gdx::test {

using namespace testing;

static const uint32_t s_rows = 80;
static const uint32_t s_cols = 60;

template <template <typename> typename Raster, typename Value>
struct RasterValuePair
{
    using value_type = Value;
    using raster     = Raster<Value>;
};

template <template <typename> typename Raster>
struct RasterType
{
    template <typename T>
    using type = Raster<T>;
};

template <typename RasterType>
class TestBase : public Test
{
public:
    static constexpr auto _nan = std::numeric_limits<typename RasterType::value_type>::quiet_NaN();

    static constexpr typename RasterType::value_type testType(double value) noexcept
    {
        return static_cast<typename RasterType::value_type>(value);
    }
};

inline void compareMetaData(const RasterMetadata& expected, const RasterMetadata& actual)
{
    EXPECT_EQ(expected.rows, actual.rows);
    EXPECT_EQ(expected.cols, actual.cols);
    EXPECT_EQ(expected.nodata, actual.nodata);
    EXPECT_DOUBLE_EQ(expected.cellSize, actual.cellSize);
    EXPECT_DOUBLE_EQ(expected.xll, actual.xll);
    EXPECT_DOUBLE_EQ(expected.yll, actual.yll);
}

using RasterTypes = testing::Types<
    RasterValuePair<MaskedRaster, int32_t>,
    RasterValuePair<MaskedRaster, int64_t>,
    RasterValuePair<MaskedRaster, float>,
    RasterValuePair<MaskedRaster, double>,
    RasterValuePair<DenseRaster, int32_t>,
    RasterValuePair<DenseRaster, float>>;

using RasterIntTypes = testing::Types<
    RasterValuePair<MaskedRaster, int32_t>,
    RasterValuePair<MaskedRaster, int64_t>,
    RasterValuePair<DenseRaster, int32_t>,
    RasterValuePair<DenseRaster, int64_t>>;

using RasterFloatTypes = testing::Types<
    RasterValuePair<MaskedRaster, float>,
    RasterValuePair<MaskedRaster, double>,
    RasterValuePair<DenseRaster, float>,
    RasterValuePair<DenseRaster, double>>;

using UnspecializedRasterTypes = testing::Types<
    RasterType<MaskedRaster>,
    RasterType<DenseRaster>>;
}
