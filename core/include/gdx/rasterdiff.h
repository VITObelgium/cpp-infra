#pragma once

#include <cinttypes>

namespace gdx {

struct RasterDiff
{
    uint32_t zeroToNonZero   = 0;
    uint32_t nonZeroToZero   = 0;
    uint32_t nodataToZero    = 0;
    uint32_t nodataToNonZero = 0;
    uint32_t zeroToNodata    = 0;
    uint32_t nonZeroToNodata = 0;
    uint32_t dataDifference  = 0;
    uint32_t equal           = 0;

    uint32_t differentCells() const noexcept
    {
        return zeroToNonZero +
               nonZeroToZero +
               nodataToZero +
               zeroToNodata +
               nodataToNonZero +
               nonZeroToNodata +
               dataDifference;
    }
};

template <typename T1, typename T2>
void diff_value(T1 v1, T2 v2, double tolerance, RasterDiff& diff)
{
    using WidestType = decltype(T1() + T2());

    bool equal = false;
    if constexpr (std::numeric_limits<WidestType>::has_quiet_NaN) {
        equal = gdx::cpu::float_equal_to<WidestType>(static_cast<WidestType>(tolerance))(static_cast<WidestType>(v1), static_cast<WidestType>(v2));
    } else {
        (void)tolerance;
        equal = static_cast<WidestType>(v1) == static_cast<WidestType>(v2);
    }

    if (equal) {
        ++diff.equal;
        return;
    }

    if (v1 == T1(0)) {
        ++diff.zeroToNonZero;
        return;
    }

    if (v2 == T2(0)) {
        ++diff.nonZeroToZero;
        return;
    }

    ++diff.dataDifference;
}

template <typename RasterType1, typename RasterType2>
RasterDiff diff_rasters(const RasterType1& r1, const RasterType2& r2, double tolerance)
{
    if (r1.size() != r2.size()) {
        throw InvalidArgument("Raster sizes do not match {}x{} vs {}x{}", r1.rows(), r1.cols(), r2.rows(), r2.cols());
    }

    using T1 = typename RasterType1::value_type;
    using T2 = typename RasterType2::value_type;

    RasterDiff diff;

    for (int32_t i = 0; i < r1.size(); ++i) {
        if (r1.is_nodata(i) != r2.is_nodata(i)) {
            if (r1.is_nodata(i)) {
                if (r2[i] == T2(0)) {
                    ++diff.nodataToZero;
                } else {
                    ++diff.nodataToNonZero;
                }
            } else {
                if (r1[i] == T1(0)) {
                    ++diff.zeroToNodata;
                } else {
                    ++diff.nonZeroToNodata;
                }
            }
        } else if (!r1.is_nodata(i) && !r2.is_nodata(i)) {
            diff_value(r1[i], r2[i], tolerance, diff);
        }
    }

    return diff;
}
}
