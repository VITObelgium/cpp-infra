#pragma once

#include "infra/cast.h"

#include <algorithm>
#include <cmath>

namespace gdx {

template <template <typename> typename RasterType, typename T>
RasterType<T> replaceValue(const RasterType<T>& ras, const T oldValue, const T newValue)
{
    RasterType<T> result(ras.metadata());

    std::transform(begin(ras), end(ras), begin(result), [oldValue, newValue](auto value) {
        if constexpr (RasterType<T>::raster_type_has_nan) {
            if (std::isfinite(value) && value == oldValue) {
                return newValue;
            }
        } else if (value == oldValue) {
            return newValue;
        }

        return value;
    });

    return result;
}

template <template <typename> typename RasterType, typename TSource, typename TDest>
RasterType<TDest> cast(const RasterType<TSource>& srcData)
{
    constexpr bool srcHasNaN = RasterType<TSource>::raster_type_has_nan;
    constexpr bool dstHasNaN = RasterType<TDest>::raster_type_has_nan;

    auto resultMeta = srcData.metadata();
    if constexpr (!dstHasNaN) {
        if (resultMeta.nodata) {
            if (std::isnan(*resultMeta.nodata)) {
                // modify the NaN nodata when the resulting type does not support NaN values
                resultMeta.nodata = static_cast<TDest>(*resultMeta.nodata);
            } else if (!inf::fits_in_type<TDest>(resultMeta.nodata.value())) {
                resultMeta.nodata = std::numeric_limits<TDest>::max();
            }
        }
    }

    RasterType<TDest> dstData(resultMeta);

    if constexpr (!srcHasNaN && dstHasNaN) {
        if (srcData.metadata().nodata) {
            auto nodata = *srcData.metadata().nodata;
            // nodata values will be replaced with nan
#pragma omp parallel for
            for (int32_t i = 0; i < srcData.size(); ++i) {
                if (srcData[i] == nodata) {
                    dstData[i] = std::numeric_limits<TDest>::quiet_NaN();
                } else {
                    dstData[i] = static_cast<TDest>(srcData[i]);
                }
            }

            return resultMeta;
        }
    } else if constexpr (srcHasNaN && !dstHasNaN) {
        if (resultMeta.nodata) {
            auto nodata = static_cast<TDest>(*resultMeta.nodata);
            // nan values need to be replaced with the nodata value
#pragma omp parallel for
            for (int32_t i = 0; i < srcData.size(); ++i) {
                if (std::isnan(srcData[i]) || srcData[i] == srcData.metadata().nodata) {
                    dstData[i] = nodata;
                } else {
                    dstData[i] = static_cast<TDest>(srcData[i]);
                }
            }

            return resultMeta;
        }
    }

    // No nodata conversions, regular copy
#pragma omp parallel for
    for (int32_t i = 0; i < srcData.size(); ++i) {
        dstData[i] = static_cast<TDest>(srcData[i]);
    }

    return resultMeta;
}
}
