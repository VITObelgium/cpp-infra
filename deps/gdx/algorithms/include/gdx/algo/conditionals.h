#pragma once

#include "infra/cast.h"

namespace gdx {

template <
    template <typename> typename RasterType1, typename T1,
    template <typename> typename RasterType2, typename T2>
RasterType2<T2> ifThen(const RasterType1<T1>& condition, const RasterType2<T2>& thenRaster)
{
    if (condition.size() != thenRaster.size()) {
        throw RuntimeError("If: Incompatible raster sizes if {}x{} then {}x{}",
            condition.rows(), condition.cols(),
            thenRaster.rows(), thenRaster.cols());
    }

    RasterType2<T2> result = thenRaster.copy();
    if (!result.nodata().has_value()) {
        result.set_nodata(std::numeric_limits<T2>::max());
    }

    for (int32_t i = 0; i < condition.size(); ++i) {
        if (!condition[i]) {
            result.mark_as_nodata(i);
        }
    }

    return result;
}

template <
    template <typename> typename RasterType1, typename T1,
    template <typename> typename RasterType2, typename T2,
    template <typename> typename RasterType3, typename T3>
auto ifThenElse(const RasterType1<T1>& condition, const RasterType2<T2>& thenRaster, const RasterType3<T3>& elseRaster)
{
    using ResultType = std::common_type_t<T2, T3>;

    if (condition.size() != thenRaster.size() || condition.size() != elseRaster.size()) {
        throw RuntimeError("If: Incompatible raster sizes if {}x{} then {}x{} else {}x{}",
            condition.rows(), condition.cols(),
            thenRaster.rows(), thenRaster.cols(),
            elseRaster.rows(), elseRaster.cols());
    }

    auto meta = thenRaster.metadata();
    if (!meta.nodata.has_value()) {
        meta.nodata = elseRaster.metadata().nodata;

        if (!meta.nodata.has_value()) {
            meta.nodata = inf::truncate<double>(std::numeric_limits<ResultType>::max());
        }
    }

    RasterType2<ResultType> result(meta);

    for (int32_t i = 0; i < condition.size(); ++i) {
        if (condition.is_nodata(i)) {
            result.mark_as_nodata(i);
            continue;
        }

        if (condition[i]) {
            if (thenRaster.is_nodata(i)) {
                result.mark_as_nodata(i);
            } else {
                result[i] = static_cast<ResultType>(thenRaster[i]);
            }
        } else {
            if (elseRaster.is_nodata(i)) {
                result.mark_as_nodata(i);
            } else {
                result[i] = static_cast<ResultType>(elseRaster[i]);
            }
        }
    }

    return result;
}
}
