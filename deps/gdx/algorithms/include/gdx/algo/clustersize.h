#pragma once

#include "gdx/algo/clusterutils.h"

#include <cassert>
#include <gsl/span>
#include <limits>
#include <vector>

namespace gdx {

template <template <typename> typename RasterType, typename T>
RasterType<int32_t> clusterSize(const RasterType<T>& ras, ClusterDiagonals diagonals)
{
    showWarningIfClusteringOnFloats(ras);

    const auto rows = ras.rows();
    const auto cols = ras.cols();

    int32_t nodata  = -9999;
    auto resultMeta = ras.metadata();
    if (resultMeta.nodata.has_value()) {
        // Use -9999 as nodata as this cannot conflict with a result from the sum
        resultMeta.nodata = nodata;
    }

    RasterType<int32_t> result(resultMeta, 0);
    RasterType<uint8_t> mark(ras.metadata(), s_markTodo);
    std::vector<Cell> clusterCells;
    FiLo<Cell> border(rows, cols);
    uint32_t clusterCount = 0;

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (ras.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            if (ras(r, c) == 0) {
                result(r, c) = 0;
            } else if (ras(r, c) > 0 && mark(r, c) == s_markTodo) {
                ++clusterCount;
                int32_t sum = 0;
                clusterCells.clear();
                border.clear();
                const auto clusterValue = ras(r, c);

                // add current cell to the cluster
                insertCell(Cell(r, c), clusterCells, mark, border);

                while (!border.empty()) {
                    auto cell            = border.pop_head();
                    mark(cell.r, cell.c) = s_markDone;
                    ++sum;

                    // add the four neighbouring cells if they have the same value
                    visitNeighbourCells(cell, rows, cols, [&](const Cell& neighbour) {
                        handleCell(neighbour, clusterValue, clusterCells, mark, border, ras);
                    });

                    if (diagonals == ClusterDiagonals::Include) {
                        visitNeighbourDiagCells(cell, rows, cols, [&](const Cell& neighbour) {
                            handleCell(neighbour, clusterValue, clusterCells, mark, border, ras);
                        });
                    }
                }

                for (auto& cell : clusterCells) {
                    result(cell.r, cell.c) = sum;
                    result.mark_as_data(cell);
                }
            }
        }
    }
    return result;
}
}
