#pragma once

#include <functional>
#include <set>
#include <tuple>

namespace gdx {

template <template <typename> typename RasterType, typename T>
RasterType<T> blurFilter(const RasterType<T>& ras)
{
    const auto rows = ras.rows();
    const auto cols = ras.cols();
    auto resultMeta = ras.metadata();

    RasterType<T> result(resultMeta);

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (ras.is_nodata(r, c)) {
                result(r, c) = ras(r, c);
                continue;
            }

            double sum = 0.0;

            int nn = 0;
            for (int rr = r - 1; rr <= r + 1; ++rr) {
                for (int cc = c - 1; cc <= c + 1; ++cc) {
                    if (resultMeta.is_on_map(rr, cc) && !ras.is_nodata(rr, cc)) {
                        sum += ras(rr, cc);
                        ++nn;
                    }
                }
            }

            result(r, c) = nn > 0 ? static_cast<T>(sum / nn) : ras(r, c);
        }
    }

    return result;
}
}
