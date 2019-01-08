#pragma once

#include "gdx/algo/algorithm.h"

#include <functional>
#include <unordered_map>

namespace gdx {

enum class FilterMode
{
    Constant,
    Linear,
    Exponential,
    Exponential2
};

template <typename RasterType>
static void increment_or_assign(RasterType& ras, int row, int col, double value)
{
    using TOutput = typename RasterType::value_type;

    if (ras.is_nodata(row, col)) {
        ras.mark_as_data(row, col);
        ras(row, col) = static_cast<TOutput>(value);
    } else {
        ras(row, col) += static_cast<TOutput>(value);
    }
}

template <typename OutputRasterType, typename InputRasterType>
OutputRasterType filter(const InputRasterType& input, FilterMode mode, int radiusInCells)
{
    OutputRasterType result(input.metadata());
    if (result.metadata().nodata.has_value()) {
        result.fill_with_nodata();
    }

    const auto numRows   = input.rows();
    const auto numCols   = input.cols();
    const int radius_sqr = radiusInCells * radiusInCells;

    for (int i = 0; i < numRows; ++i) {
        for (int j = 0; j < numCols; ++j) {
            if (input.is_nodata(i, j)) {
                continue;
            }

            auto value = input(i, j);

            int row = i;
            int col = j;
            int r0  = std::max(0, row - radiusInCells);
            int r1  = std::min(row + radiusInCells, numRows - 1);
            int c0  = std::max(0, col - radiusInCells);
            int c1  = std::min(col + radiusInCells, numCols - 1);

            double sum_area = 0;
            for (int r = r0; r <= r1; ++r) {
                const int dr     = (r - row);
                const int dr_sqr = dr * dr;
                for (int c = c0; c <= c1; ++c) {
                    const int dc     = (c - col);
                    const int dc_sqr = dc * dc;

                    const int d_sqr = dr_sqr + dc_sqr;
                    if (d_sqr <= radius_sqr) {
                        if (mode == FilterMode::Exponential2) {
                            if (d_sqr > 0) {
                                sum_area += (1.0 / d_sqr);
                            } else {
                                sum_area += 1.0;
                            }
                        } else if (mode == FilterMode::Exponential) {
                            if (d_sqr > 0) {
                                sum_area += (1.0 / std::sqrt((double)d_sqr));
                            } else {
                                sum_area += 1.0;
                            }
                        } else if (mode == FilterMode::Linear) {
                            sum_area += 1.0 - std::sqrt((double)d_sqr) / (radiusInCells + 1);
                        } else {
                            assert(mode == FilterMode::Constant);
                            sum_area += 1.0;
                        }
                    }
                }
            }

            for (int r = r0; r <= r1; ++r) {
                const int dr     = (r - row);
                const int dr_sqr = dr * dr;
                for (int c = c0; c <= c1; ++c) {
                    const int dc     = (c - col);
                    const int dc_sqr = dc * dc;

                    const int d_sqr = dr_sqr + dc_sqr;
                    if (d_sqr <= radius_sqr) {
                        if (mode == FilterMode::Exponential2) {
                            if (d_sqr > 0) {
                                increment_or_assign(result, r, c, (value * (1.0 / d_sqr)) / sum_area);
                            } else {
                                increment_or_assign(result, r, c, value / sum_area);
                            }
                        } else if (mode == FilterMode::Exponential) {
                            if (d_sqr > 0) {
                                increment_or_assign(result, r, c, (value * (1.0 / std::sqrt(static_cast<double>(d_sqr)))) / sum_area);
                            } else {
                                result(r, c) += (value * (1.0)) / sum_area;
                                increment_or_assign(result, r, c, value / sum_area);
                            }
                        } else if (mode == FilterMode::Linear) {
                            increment_or_assign(result, r, c, (value * (1.0 - std::sqrt(static_cast<double>(d_sqr)) / (radiusInCells + 1))) / sum_area);
                        } else {
                            assert(mode == FilterMode::Constant);
                            increment_or_assign(result, r, c, value / sum_area);
                        }
                    }
                }
            }
        }
    }

    return result;
}

template <typename OutputRasterType, typename InputRasterType, typename AreaCallback>
OutputRasterType filter_circular(const InputRasterType& input, int radiusInCells, AreaCallback&& cb)
{
    OutputRasterType result(input.metadata());
    if (result.metadata().nodata.has_value()) {
        result.fill_with_nodata();
    }

    gdx::transform(input, result, [&input, radiusInCells, &cb](auto& inputValue) {
        return cb(neighbouring_cells_circular(input, &inputValue, radiusInCells));
    });

    return result;
}

template <typename OutputRasterType, typename InputRasterType, typename AreaCallback>
OutputRasterType filter_square(const InputRasterType& input, int radiusInCells, AreaCallback&& cb)
{
    OutputRasterType result(input.metadata());
    if (result.metadata().nodata.has_value()) {
        result.fill_with_nodata();
    }

    gdx::transform(input, result, [&input, radiusInCells, &cb](auto& inputValue) {
        return cb(neighbouring_cells_square(input, &inputValue, radiusInCells));
    });

    return result;
}

template <typename OutputRasterType, typename InputRasterType>
OutputRasterType average_filter_square(const InputRasterType& input, int radiusInCells)
{
    using T = typename OutputRasterType::value_type;

    OutputRasterType result(input.metadata());
    if (result.metadata().nodata.has_value()) {
        result.fill_with_nodata();
    }

    std::transform(optional_value_begin(input), optional_value_end(input), optional_value_begin(result), [&input, radiusInCells](auto& value) -> std::optional<T> {
        auto area     = cells_square(input, &(*value), radiusInCells);
        T sum         = 0;
        int32_t count = 0;
        for (auto& value : area) {
            sum += value;
            ++count;
        }

        if (count == 0 && !value) {
            // Complete nodata area: output will be nodata
            return std::optional<T>();
        }

        return static_cast<T>(double(sum) / count);
    });

    return result;
}
}
