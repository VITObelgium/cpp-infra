#pragma once

#include "gdx/algo/clusterutils.h"
#include "gdx/exception.h"

#include <cassert>
#include <gsl/span>
#include <limits>
#include <map>
#include <vector>

namespace gdx {

template <template <typename> typename RasterType, typename T>
RasterType<int32_t> clusterId(const RasterType<T>& ras, ClusterDiagonals diagonals)
{
    showWarningIfClusteringOnFloats(ras);

    const auto rows = ras.rows();
    const auto cols = ras.cols();

    auto resultMeta = ras.metadata();
    // use -9999 as nodata as it cannot clash with a cluster id
    int32_t nodata = -9999;
    if (resultMeta.nodata.has_value()) {
        resultMeta.nodata = nodata;
    }

    RasterType<int32_t> result(resultMeta);
    RasterType<uint8_t> mark(ras.rows(), ras.cols(), s_markTodo);
    std::vector<Cell> clusterCells;
    FiLo<Cell> border(rows, cols);

    int32_t clusterId = 0;
    for (int32_t r = 0; r < ras.rows(); ++r) {
        for (int32_t c = 0; c < ras.cols(); ++c) {
            if (ras.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            if (ras(r, c) == 0) {
                result(r, c) = 0;
            } else if (ras(r, c) > 0 && mark(r, c) == s_markTodo) {
                ++clusterId;

                clusterCells.clear();
                border.clear();

                const auto clusterValue = ras(r, c);

                // add current cell to the cluster
                insertCell(Cell(r, c), clusterCells, mark, border);

                while (!border.empty()) {
                    auto cell = border.pop_head();

                    visitNeighbourCells(cell, rows, cols, [&](const Cell& neighbour) {
                        handleCell(neighbour, clusterValue, clusterCells, mark, border, ras);
                    });

                    if (diagonals == ClusterDiagonals::Include) {
                        visitNeighbourDiagCells(cell, rows, cols, [&](const Cell& neighbour) {
                            handleCell(neighbour, clusterValue, clusterCells, mark, border, ras);
                        });
                    }
                }

                for (const auto& cell : clusterCells) {
                    mark(cell.r, cell.c)   = s_markDone;
                    result(cell.r, cell.c) = clusterId;
                }
            }
        }
    }

    return result;
}

template <template <typename> typename RasterType, typename T>
RasterType<int32_t> fuzzyClusterId(const RasterType<T>& ras, float radiusInMeter)
{
    const auto rows = ras.rows();
    const auto cols = ras.cols();

    float radius             = radiusInMeter / static_cast<float>(ras.metadata().cellSize);
    const auto radiusInCells = static_cast<int>(radius);
    const auto radius2       = static_cast<int32_t>(radius * radius);

    auto resultMeta = ras.metadata();
    // use -9999 as nodata as it cannot clash with a cluster id
    int32_t nodata = -9999;
    if (resultMeta.nodata.has_value()) {
        resultMeta.nodata = nodata;
    }

    RasterType<int32_t> result(resultMeta);
    RasterType<uint8_t> mark(ras.rows(), ras.cols(), s_markDone);

    for (int32_t i = 0; i < ras.size(); ++i) {
        if (ras.is_nodata(i)) {
            mark[i] = s_markDone;
            result.mark_as_nodata(i);
            continue;
        }

        if (ras[i] > 0) {
            mark[i] = s_markTodo;
        } else {
            result[i] = 0;
        }
    }

    int32_t clusterId = 0;
    FiLo<Cell> border(rows, cols);

    for (int32_t r = 0; r < ras.rows(); ++r) {
        for (int32_t c = 0; c < ras.cols(); ++c) {
            if (mark(r, c) == s_markTodo) {
                ++clusterId;

                border.clear();
                border.push_back(Cell(r, c));
                mark(r, c) = s_markBorder;
                while (!border.empty()) {
                    auto cell              = border.pop_head();
                    mark(cell.r, cell.c)   = s_markDone;
                    result(cell.r, cell.c) = clusterId;

                    const int r0 = (cell.r - radiusInCells < 0 ? 0 : cell.r - radiusInCells);
                    const int c0 = (cell.c - radiusInCells < 0 ? 0 : cell.c - radiusInCells);
                    const int r1 = (cell.r + radiusInCells > rows - 1 ? rows - 1 : cell.r + radiusInCells);
                    const int c1 = (cell.c + radiusInCells > cols - 1 ? cols - 1 : cell.c + radiusInCells);

                    for (int32_t rr = r0; rr <= r1; ++rr) {
                        const auto dr  = rr - cell.r;
                        const auto dr2 = dr * dr;

                        auto markIndex = rr * cols + c0;
                        for (int32_t cc = c0; cc <= c1; ++markIndex, ++cc) {
                            if (mark[markIndex] == s_markTodo) {
                                const int dc = cc - cell.c;
                                if (dr2 + dc * dc <= radius2) {
                                    mark[markIndex] = s_markBorder;
                                    border.push_back(Cell(rr, cc));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return result;
}

template <template <typename> typename RasterType>
void handleCellWithObstaclesStraight(const Cell cell,
    const RasterType<int32_t>& catMap,
    const int clusterValue,
    const RasterType<uint8_t>& obstacleMap,
    RasterType<uint8_t>& mark,
    FiLo<Cell>& border)
{
    if ((catMap[cell] == clusterValue) && (mark[cell] == s_markTodo)) {
        if (!obstacleMap[cell]) {
            insertCell(cell, mark, border);
        }
    }
}

template <template <typename> typename RasterType>
void handleCellWithObstaclesDiag(const Cell oldCell, const Cell cell,
    const RasterType<int32_t>& catMap,
    const int clusterValue,
    const RasterType<uint8_t>& obstacleMap,
    RasterType<uint8_t>& mark,
    FiLo<Cell>& border)
{
    if ((catMap[cell] == clusterValue) && (mark[cell] == s_markTodo)) {
        if ((!obstacleMap[cell]) && ((!obstacleMap(oldCell.r, cell.c)) || (!obstacleMap(cell.r, oldCell.c)))) {
            insertCell(cell, mark, border);
        }
    }
}

template <template <typename> typename RasterType>
void computeClusterIdOfObstacleCell(int32_t row, int32_t col, RasterType<int32_t>& clusterIdMap, const RasterType<uint8_t>& obstacleMap, std::vector<int32_t>& clusterSize)
{
    const auto rows = clusterIdMap.rows();
    const auto cols = clusterIdMap.cols();

    // assign to cluster with largest nr of neighbors
    // in case of equal number of neighbors, assign to SMALLEST neighboring cluster
    std::map<int32_t, int32_t> countNeighbors; // neighboring clusterId --> nr of neighbors
    for (int32_t r = row - 1; r <= row + 1; ++r) {
        for (int32_t c = col - 1; c <= col + 1; ++c) {
            if (0 <= r && r < rows && 0 <= c && c < cols) {
                if (!obstacleMap(r, c)) {
                    const int clusterId = clusterIdMap(r, c);
                    if (clusterId > 0) {
                        const auto& iter = countNeighbors.find(clusterId);
                        if (iter != countNeighbors.end()) {
                            ++iter->second;
                        } else {
                            countNeighbors[clusterId] = 1;
                        }
                    }
                }
            }
        }
    }

    int clusterId = -1;
    int mostCount = 9;

    for (auto iter = countNeighbors.begin(); iter != countNeighbors.end(); ++iter) {
        if (clusterId == -1 || mostCount < iter->second) {
            clusterId = iter->first;
            mostCount = iter->second;
        } else if ((mostCount == iter->second) && (clusterSize[clusterId] > clusterSize[iter->first])) {
            clusterId = iter->first;
        }
    }

    if (clusterId > 0) {
        clusterIdMap(row, col) = clusterId;
        ++clusterSize[clusterId];
    }
}

template <template <typename> typename RasterType, typename T>
RasterType<int32_t> clusterIdWithObstacles(const RasterType<T>& raster,
    const RasterType<int32_t>& catMap,
    const RasterType<uint8_t>& obstacleMap)
{
    if (raster.size() != catMap.size() || raster.size() != obstacleMap.size()) {
        throw InvalidArgument("Raster, cathegory and obstacle map dimensions should be the same");
    }

    const auto rows = raster.rows();
    const auto cols = raster.cols();

    auto resultMeta = raster.metadata();
    // use -9999 as nodata as it cannot clash with a cluster id
    int32_t nodata = -9999;
    if (resultMeta.nodata.has_value()) {
        resultMeta.nodata = nodata;
    }

    RasterType<int32_t> result(resultMeta, nodata);
    RasterType<uint8_t> mark(rows, cols, s_markTodo);

    int32_t clusterId = 0;
    FiLo<Cell> border(rows, cols);

    std::vector<int32_t> clusterSize(rows * cols, 0);
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (catMap(r, c) > 0 && mark(r, c) == s_markTodo && !obstacleMap(r, c)) {
                ++clusterId;
                border.clear();
                const auto clusterValue = catMap(r, c);
                insertCell(Cell(r, c), mark, border);

                while (!border.empty()) {
                    auto cell            = border.pop_head();
                    mark(cell.r, cell.c) = s_markDone;
                    result[cell]         = clusterId;
                    result.mark_as_data(cell);

                    visitNeighbourCells(cell, rows, cols, [&](const Cell& neighbour) {
                        handleCellWithObstaclesStraight(neighbour, catMap, clusterValue, obstacleMap, mark, border);
                    });

                    visitNeighbourDiagCells(cell, rows, cols, [&](const Cell& neighbour) {
                        handleCellWithObstaclesDiag(cell, neighbour, catMap, clusterValue, obstacleMap, mark, border);
                    });
                }
            }
        }
    }

    // give remaining cells under obstacles a clusterId
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (catMap(r, c) > 0 && obstacleMap(r, c)) {
                assert(mark(r, c) == s_markTodo);
                computeClusterIdOfObstacleCell(r, c, result, obstacleMap, clusterSize);
            }
        }
    }

    return result;
}
}
