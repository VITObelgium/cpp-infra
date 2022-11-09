#include "infra/legenddataanalyser.h"
#include "infra/algo.h"
#include "infra/gdal.h"
#include "infra/test/printsupport.h"

#include <doctest/doctest.h>
#include <numeric>

namespace inf::test {

TEST_CASE("LegendDataAnalyserTest.linear")
{
    auto ds = gdal::RasterDataSet::open(fs::u8path(TEST_DATA_DIR) / "raster.tif");

    auto data = ds.read_rasterdata<float>(1);
    if (auto nodata = ds.nodata_value(1); nodata.has_value()) {
        inf::remove_value_from_container(data, truncate<float>(*nodata));
    }

    auto [minIter, maxIter] = std::minmax_element(data.begin(), data.end());

    LegendDataAnalyser analyser(data);
    analyser.set_number_of_classes(5);
    analyser.calculate_classbounds(LegendScaleType::Linear, *minIter, *maxIter);
    auto bounds = analyser.classbounds();

    analyser.calculate_classbounds(LegendScaleType::LinearNoOutliers, *minIter, *maxIter);
    auto boundsNoOutliers = analyser.classbounds();

    CHECK(bounds.size() == 5);
    CHECK(boundsNoOutliers.size() == 5);

    CHECK(std::get<0>(boundsNoOutliers.front()) >= std::get<0>(bounds.front()));
    CHECK(std::get<1>(boundsNoOutliers.back()) <= std::get<1>(bounds.back()));
}

}
