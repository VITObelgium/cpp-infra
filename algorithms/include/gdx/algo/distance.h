#pragma once

#include "gdx/cell.h"
#include "gdx/exception.h"
#include "gdx/log.h"

#include "gdx/algo/clusterutils.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <gsl/span>
#include <limits>
#include <vector>

namespace gdx {

// on whole maps: targets and barriers outside modelling area do matter in case region map reduction is used instead zoning.
// for (int r = 0; r < gc->nRows; ++r) {
//     for (int c = 0; c < gc->nCols; ++c) {
//         if (ftarget(r, c) && !isnan(ftarget(r, c))) btarget(r, c) = 1;
//         if (fbarrier(r, c) || isnan(fbarrier(r, c))) bbarrier(r, c) = 1;
//     }
// }

namespace internal {

template <template <typename> typename RasterType, typename T>
void handleCellClosestTarget(float deltaD, const Cell& cell, const Cell& newCell,
    RasterType<float>& distanceToTarget,
    RasterType<T>& closesttarget,
    RasterType<uint8_t>& mark,
    FiLo<Cell>& border)
{
    if (distanceToTarget[newCell] > distanceToTarget[cell] + deltaD) {
        distanceToTarget[newCell] = distanceToTarget[cell] + deltaD;
        closesttarget[newCell]    = closesttarget[cell];
        if (mark[newCell] != s_markBorder) {
            mark[newCell] = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType, typename T>
void handleTimeCell(float deltaD, const Cell& cell, const Cell& newCell,
    RasterType<float>& distanceToTarget,
    RasterType<uint8_t>& mark,
    const RasterType<T>& travelTime,
    FiLo<Cell>& border)
{
    if (distanceToTarget.is_nodata(cell) || distanceToTarget.is_nodata(newCell)) {
        return;
    }
    const float alternativeDist = static_cast<float>(distanceToTarget[cell] + deltaD * travelTime[newCell]);
    float& d                    = distanceToTarget[newCell];
    if (d > alternativeDist) {
        d          = alternativeDist;
        uint8_t& m = mark[newCell];
        if (m != s_markBorder) {
            m = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType, typename T>
void handleSumLeTimeDistanceCell(float deltaD, const Cell& cell, const Cell& newCell,
    RasterType<float>& distanceToTarget,
    RasterType<uint8_t>& mark,
    const RasterType<T>& travelTime,
    FiLo<Cell>& border,
    std::vector<Cell>& cells)
{
    if (travelTime.is_nodata(newCell)) {
        return;
    }
    const float alternativeDist = static_cast<float>(distanceToTarget[cell] +
                                                     deltaD / 2.0f * (travelTime[cell] + travelTime[newCell]));
    float& d                    = distanceToTarget[newCell];
    if (d > alternativeDist) {
        d          = alternativeDist;
        uint8_t& m = mark[newCell];
        if (m != s_markBorder) {
            if (m == s_markTodo) {
                cells.push_back(newCell);
            }
            m = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType, typename T>
void handleCellValueAtClosestTarget(float deltaD, const Cell& cell, const Cell& newCell,
    RasterType<float>& distanceToTarget,
    RasterType<uint8_t>& mark,
    RasterType<T>& valueatclosesttarget,
    FiLo<Cell>& border)
{
    if (distanceToTarget[newCell] > distanceToTarget[cell] + deltaD) {
        distanceToTarget[newCell]     = distanceToTarget[cell] + deltaD;
        valueatclosesttarget[newCell] = valueatclosesttarget[cell];
        if (mark[newCell] != s_markBorder) {
            mark[newCell] = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType, typename TTravel, typename TValue>
void handleCellValueAtClosestTravelTarget(float deltaD, const Cell& cell, const Cell& newCell,
    RasterType<float>& distanceToTarget,
    RasterType<TValue>& valueatclosesttarget,
    const RasterType<TTravel>& travelTime,
    RasterType<uint8_t>& mark,
    FiLo<Cell>& border)
{
    auto alternativeDist = static_cast<float>(distanceToTarget[cell] + deltaD * travelTime[newCell]);
    if (distanceToTarget[newCell] > alternativeDist) {
        distanceToTarget[newCell]     = alternativeDist;
        valueatclosesttarget[newCell] = valueatclosesttarget[cell];
        if (mark[newCell] != s_markBorder) {
            mark[newCell] = s_markBorder;
            border.push_back(newCell);
        }
    }
}

template <template <typename> typename RasterType>
void handleCell(float deltaD, const Cell& cell, const Cell& newCell,
    RasterType<float>& distanceToTarget,
    RasterType<uint8_t>& mark,
    FiLo<Cell>& border)
{
    if (distanceToTarget[newCell] > distanceToTarget[cell] + deltaD) {
        distanceToTarget[newCell] = distanceToTarget[cell] + deltaD;
        if (mark[newCell] != s_markBorder) {
            mark[newCell] = s_markBorder;
            border.push_back(newCell);
        }
    }
}
}

template <template <typename> typename RasterType>
RasterType<float> distance(const RasterType<uint8_t>& target)
{
    const auto rows         = target.rows();
    const auto cols         = target.cols();
    const float cellSize    = static_cast<float>(target.metadata().cellSize);
    const float unreachable = std::numeric_limits<float>::max() / (2.f * cellSize);

    auto meta   = target.metadata();
    meta.nodata = RasterType<float>::NaN;
    RasterType<float> distanceToTarget(std::move(meta), unreachable);
    RasterType<uint8_t> mark(target.rows(), target.cols(), s_markTodo);

    FiLo<Cell> border(rows, cols);

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (target.is_nodata(r, c)) {
                distanceToTarget.mark_as_nodata(r, c);
            } else if (target(r, c) != 0) {
                distanceToTarget(r, c) = 0;
                mark(r, c)             = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visitNeighbourCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleCell(1.f, cell, neighbour, distanceToTarget, mark, border);
        });

        visitNeighbourDiagCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleCell(sqrt2, cell, neighbour, distanceToTarget, mark, border);
        });
    }

    distanceToTarget *= static_cast<float>(target.metadata().cellSize);
    return distanceToTarget;
}

//convertNodata(travelTime, gc->nRows, gc->nCols, FLT_MAX / 4); // FLT_MAX/4 allows to add 2 x sqrt(2) of them and stell be lesst than FLT_MAX

template <template <typename> typename RasterType, typename T>
RasterType<float> travelDistance(const RasterType<uint8_t>& target, const RasterType<T>& travelTime)
{
    if (target.size() != travelTime.size()) {
        throw InvalidArgument("Target raster dimensions should match travel time raster dimensions");
    }

    const auto rows         = target.rows();
    const auto cols         = target.cols();
    const float unreachable = std::numeric_limits<float>::max();

    auto meta   = target.metadata();
    meta.nodata = RasterType<float>::NaN;
    RasterType<float> distanceToTarget(std::move(meta), unreachable);
    RasterType<uint8_t> mark(target.metadata(), s_markTodo);

    FiLo<Cell> border(rows, cols);

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (target.is_nodata(r, c) || travelTime.is_nodata(r, c)) {
                distanceToTarget.mark_as_nodata(r, c);
            } else if (target(r, c) != 0) {
                distanceToTarget(r, c) = 0;
                mark(r, c)             = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visitNeighbourCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleTimeCell(1.f, cell, neighbour, distanceToTarget, mark, travelTime, border);
        });

        visitNeighbourDiagCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleTimeCell(sqrt2, cell, neighbour, distanceToTarget, mark, travelTime, border);
        });
    }

    return distanceToTarget;
}

template <template <typename> typename RasterType, typename T>
RasterType<T> closestTarget(const RasterType<T>& target)
{
    const auto rows         = target.rows();
    const auto cols         = target.cols();
    const float unreachable = std::numeric_limits<float>::max();

    auto meta = target.metadata();
    meta.nodata.reset();
    RasterType<float> distanceToTarget(meta, unreachable);
    RasterType<T> closestTarget(meta, 0);
    RasterType<uint8_t> mark(meta, s_markTodo);

    FiLo<Cell> border(rows, cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (target.is_nodata(r, c)) {
                continue;
            } else if (target(r, c)) {
                distanceToTarget(r, c) = 0;
                closestTarget(r, c)    = target(r, c);
                mark(r, c)             = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visitNeighbourCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleCellClosestTarget(1.f, cell, neighbour, distanceToTarget, closestTarget, mark, border);
        });

        visitNeighbourDiagCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleCellClosestTarget(sqrt2, cell, neighbour, distanceToTarget, closestTarget, mark, border);
        });
    }

    return closestTarget;
}

template <template <typename> typename RasterType, typename TValue, typename TTarget>
RasterType<TValue> valueAtClosestTarget(const RasterType<TTarget>& target, const RasterType<TValue>& value)
{
    if (target.size() != value.size()) {
        throw InvalidArgument("Target raster dimensions should match value raster dimensions");
    }

    const auto rows         = target.rows();
    const auto cols         = target.cols();
    const float unreachable = static_cast<float>(rows * cols + 1);

    RasterType<TValue> valueAtClosestTarget(value.metadata(), 0);
    RasterType<float> distanceToTarget(value.metadata(), unreachable);

    RasterType<uint8_t> mark(target.metadata(), s_markTodo);
    FiLo<Cell> border(rows, cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (target.is_nodata(r, c)) {
                continue;
            }

            if (target(r, c)) {
                distanceToTarget(r, c) = 0;

                if (value.is_nodata(r, c)) {
                    valueAtClosestTarget.mark_as_nodata(r, c);
                } else {
                    valueAtClosestTarget(r, c) = value(r, c);
                }

                mark(r, c) = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visitNeighbourCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleCellValueAtClosestTarget(1.f, cell, neighbour, distanceToTarget, mark, valueAtClosestTarget, border);
        });

        visitNeighbourDiagCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleCellValueAtClosestTarget(sqrt2, cell, neighbour, distanceToTarget, mark, valueAtClosestTarget, border);
        });
    }

    return valueAtClosestTarget;
}

template <template <typename> typename RasterType, typename TValue, typename TTravel, typename TTarget>
RasterType<TValue> valueAtClosestTravelTarget(const RasterType<TTarget>& target, const RasterType<TTravel>& travelTimes, const RasterType<TValue>& value)
{
    if (target.size() != value.size() || target.size() != value.size()) {
        throw InvalidArgument("Target, traveltimes and value map dimensions should be the same");
    }

    const auto rows         = target.rows();
    const auto cols         = target.cols();
    const float unreachable = std::numeric_limits<float>::max();

    RasterType<TValue> valueAtClosestTarget(value.metadata(), 0);
    RasterType<float> distanceToTarget(value.metadata(), unreachable);

    RasterType<uint8_t> mark(target.metadata(), s_markTodo);
    FiLo<Cell> border(rows, cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (value.is_nodata(r, c)) {
                valueAtClosestTarget(r, c) = value(r, c);
                distanceToTarget.mark_as_nodata(r, c);
            } else if (target(r, c)) {
                distanceToTarget(r, c)     = 0;
                valueAtClosestTarget(r, c) = value(r, c);
                mark(r, c)                 = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visitNeighbourCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleCellValueAtClosestTravelTarget(1.f, cell, neighbour, distanceToTarget, valueAtClosestTarget, travelTimes, mark, border);
        });

        visitNeighbourDiagCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleCellValueAtClosestTravelTarget(sqrt2, cell, neighbour, distanceToTarget, valueAtClosestTarget, travelTimes, mark, border);
        });
    }

    return valueAtClosestTarget;
}

template <template <typename> typename RasterType, typename TValue, typename TTravel, typename TTarget>
RasterType<TValue> valueAtClosestLessThenTravelTarget(const RasterType<TTarget>& target, const RasterType<TTravel>& travelTimes, const float maxTravelTime, const RasterType<TValue>& value)
{
    if (target.size() != value.size() || target.size() != value.size()) {
        throw InvalidArgument("Target, traveltimes and value map dimensions should be the same");
    }

    const auto rows         = target.rows();
    const auto cols         = target.cols();
    const float unreachable = maxTravelTime;

    RasterType<TValue> valueAtClosestTarget(value.metadata(), 0);
    RasterType<float> distanceToTarget(value.metadata(), unreachable);

    RasterType<uint8_t> mark(target.metadata(), s_markTodo);
    FiLo<Cell> border(rows, cols);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (value.is_nodata(r, c)) {
                valueAtClosestTarget(r, c) = value(r, c);
                distanceToTarget.mark_as_nodata(r, c);
            } else if (target(r, c)) {
                distanceToTarget(r, c)     = 0;
                valueAtClosestTarget(r, c) = value(r, c);
                mark(r, c)                 = s_markBorder;
                border.push_back(Cell(r, c));
            }
        }
    }

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto cell = border.pop_head();
        assert(mark[cell] == s_markBorder);
        mark[cell] = s_markDone;

        visitNeighbourCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleCellValueAtClosestTravelTarget(1.f, cell, neighbour, distanceToTarget, valueAtClosestTarget, travelTimes, mark, border);
        });

        visitNeighbourDiagCells(cell, rows, cols, [&](const Cell& neighbour) {
            internal::handleCellValueAtClosestTravelTarget(sqrt2, cell, neighbour, distanceToTarget, valueAtClosestTarget, travelTimes, mark, border);
        });
    }

    return valueAtClosestTarget;
}

// computes the sum of the valueToSum that is within the distance via lowest travelTime
template <template <typename> typename RasterType, typename TTravel, typename TValue>
TValue computeSumLeTimeDistance(Cell targetCell,
    const RasterType<TTravel>& travelTime,
    const float maxTravelTime,
    const float unreachable,
    const RasterType<TValue>& valueToSum,
    bool inclAdjacent,
    // temporary variables internal to computeSumLeTimeDistance.  Pass them again and again instead of having to recreate them again and again.
    RasterType<float>& distanceToTarget, // expected to be all unreachable.  This function restores any changes upon return.
    RasterType<uint8_t>& mark,           // expected to be all s_markTodo.  This function restores any changes upon return.
    FiLo<Cell>& border,                  // expected to be empty.  Restored to empty upon return.
    std::vector<Cell>& cells,            // idem
    std::vector<Cell>& adjacentCells     // idem
)
{
    // preconditions on parameters :
    // distanceToTarget all unreachable
    // mark all s_markTodo
    // border empty
    // cells empty

    TValue sum = 0;

    const auto rows = mark.rows();
    const auto cols = mark.cols();

    Cell cell              = targetCell;
    distanceToTarget[cell] = 0;
    if (!travelTime.is_nodata(cell)) {
        border.push_back(cell);
        mark[cell] = s_markBorder;
    } else {
        mark[cell] = s_markDone;
    }
    cells.push_back(cell);

    const float sqrt2 = std::sqrt(2.f);
    while (!border.empty()) {
        auto curCell = border.pop_head();
        assert(mark[curCell] == s_markBorder);
        mark[curCell] = s_markDone;

        visitNeighbourCells(curCell, rows, cols, [&](const Cell& neighbour) {
            internal::handleSumLeTimeDistanceCell(1.f, curCell, neighbour, distanceToTarget, mark, travelTime, border, cells);
        });

        visitNeighbourDiagCells(curCell, rows, cols, [&](const Cell& neighbour) {
            internal::handleSumLeTimeDistanceCell(sqrt2, curCell, neighbour, distanceToTarget, mark, travelTime, border, cells);
        });
    }

    // cells contain all the locations at <= radius now.
    // All modified locations can be restored to their original state efficiently now.
    for (int i = 0; i < int(cells.size()); ++i) {
        const int r = cells[i].r;
        const int c = cells[i].c;
        assert(distanceToTarget(r, c) <= maxTravelTime);
        if (!valueToSum.is_nodata(r, c)) {
            sum += valueToSum(r, c);
        }
        assert(mark(r, c) == s_markDone);
        mark(r, c) = s_markTodo;
    }

    if (inclAdjacent) {
        assert(adjacentCells.size() == 0);
        auto handleAdjacent = [&](int rr, int cc) {
            if (0 <= rr && rr < rows && 0 <= cc && cc < cols) {
                if (distanceToTarget(rr, cc) > maxTravelTime && mark(rr, cc) == s_markTodo) {
                    if (!valueToSum.is_nodata(rr, cc)) {
                        sum += valueToSum(rr, cc);
                    }
                    mark(rr, cc) = s_markDone;
                    adjacentCells.push_back(Cell(rr, cc));
                }
            }
        };
        for (int i = 0; i < int(cells.size()); ++i) {
            const int r = cells[i].r;
            const int c = cells[i].c;
            handleAdjacent(r - 1, c);
            handleAdjacent(r + 1, c);
            handleAdjacent(r, c - 1);
            handleAdjacent(r, c + 1);
        }
    }
    for (int i = 0; i < int(cells.size()); ++i) {
        distanceToTarget[cells[i]] = unreachable;
    }
    for (int i = 0; i < int(adjacentCells.size()); ++i) {
        mark[adjacentCells[i]] = s_markTodo;
    }
    cells.clear();
    adjacentCells.clear();
    return sum;
}

template <template <typename> typename RasterType, typename TMask, typename TResistence, typename TValue>
RasterType<TValue> sumWithinTravelDistance(const RasterType<TMask>& mask,
    const RasterType<TResistence>& resistenceMap,
    const RasterType<TValue>& valueMap,
    float maxResistance,
    bool includeAdjacent)
{
    //convertNodata(mask, gc->nRows, gc->nCols, 0);
    //convertNodata(resistenceMap, gc->nRows, gc->nCols, FLT_MAX / 4); // FLT_MAX/4 allows to add 2 x sqrt(2) of them and stell be lesst than FLT_MAX // allow
    //convertNodata(valueToSumMap, gc->nRows, gc->nCols, 0);

    if (mask.size() != resistenceMap.size() || mask.size() != valueMap.size()) {
        throw InvalidArgument("Mask, resistence and value map dimensions should be the same");
    }
    if (maxResistance <= 0) {
        throw InvalidArgument("maxResistance should be postive");
    }

    const auto rows = mask.rows();
    const auto cols = mask.cols();

    auto resultMeta = valueMap.metadata();

    RasterType<TValue> result(resultMeta, 0);

    const float unreachable = std::nextafter(maxResistance, std::numeric_limits<float>::max());

    // temporary variables internal to computeSumLeTimeDistance.  Pass them again and again instead of having to recreate them again and again.
    RasterType<float> distanceToTarget(mask.metadata(), unreachable);
    RasterType<uint8_t> mark(mask.metadata(), s_markTodo);
    FiLo<Cell> border(rows, cols);
    std::vector<Cell> cells;
    std::vector<Cell> adjacentCells;

    using namespace std::chrono_literals;
    auto t00     = std::chrono::steady_clock::now();
    auto lastMsg = t00;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (!mask.is_nodata(r, c) && mask(r, c) != 0) {
                result(r, c) = computeSumLeTimeDistance(Cell(r, c), resistenceMap, maxResistance,
                    unreachable, valueMap, includeAdjacent, distanceToTarget, mark, border, cells, adjacentCells);
            }
        }

        auto now = std::chrono::steady_clock::now();
        if ((now - lastMsg) > 3s) {
            auto elapsed = now - t00;
            auto total   = elapsed * static_cast<double>(rows) / (r + 1);

            lastMsg = now;
            Log::warn("sumWithinTravelDistance processed {}%, elapsed {:02}:{:02}:{:02}, expected total runtime {:02}:{:02}:{:02}",
                100.0 * (r + 1) / rows,
                std::chrono::duration_cast<std::chrono::hours>(elapsed).count(),
                std::chrono::duration_cast<std::chrono::minutes>(elapsed).count(),
                std::chrono::duration_cast<std::chrono::seconds>(elapsed).count(),
                std::chrono::duration_cast<std::chrono::hours>(total).count(),
                std::chrono::duration_cast<std::chrono::minutes>(total).count(),
                std::chrono::duration_cast<std::chrono::seconds>(total).count());
        }
    }

    return result;
}
}
