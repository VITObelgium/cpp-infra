#pragma once

#include "gdx/algo/cast.h"
#include "gdx/cell.h"
#include "infra/log.h"

#include <array>
#include <cmath>
#include <functional>
#include <mutex>
#include <optional>
#include <set>
#include <tuple>
#include <vector>

namespace gdx {

namespace detail {
bool cellsAreNeigbours(Cell c1, Cell c2);
Cell getNeighbour(uint8_t direction, Cell cell);
uint8_t getDirectionToNeighbour(Cell cell, Cell neighbourCell);

struct Offset
{
    int8_t x;
    int8_t y;
};

static constexpr Offset lookupOffsets[] = {
    {0, 0},   // 0, should not be looked up
    {-1, 1},  // 1
    {0, 1},   // 2
    {1, 1},   // 3
    {-1, 0},  // 4
    {0, 0},   // 5 pit, should not be looked up
    {1, 0},   // 6
    {-1, -1}, // 7
    {0, -1},  // 8
    {1, -1},  // 9
};

template <template <typename> typename RasterType>
int16_t getNeighbourValue(uint8_t direction, Cell cell, const RasterType<uint8_t>& lddMap)
{
    auto neighbour = getNeighbour(direction, cell);

    if (lddMap.metadata().is_on_map(neighbour)) {
        return lddMap(neighbour.r, neighbour.c);
    }

    return -1;
}

template <template <typename> typename RasterType>
Cell getDestinationCell(Cell cell, const RasterType<uint8_t>& lddMap)
{
    auto dir = lddMap(cell.r, cell.c);
    return getNeighbour(dir, cell);
}

template <template <typename> typename RasterType, typename VisitCb>
void traverse_ldd(const Cell cell, const RasterType<uint8_t>& lddMap, VisitCb&& visitCb)
{
    assert(!lddMap.is_nodata(cell));
    uint8_t dir = lddMap[cell];

    auto curCell = cell;

    std::set<Cell> traversedCells;

    while (dir != 5) {
        traversedCells.insert(curCell);

        if (9 < dir) {
            throw RuntimeError("ldd map is unsound: ldd value outside [0..9] {}", curCell);
        }

        const Offset& offset = lookupOffsets[dir];
        const auto destCell  = Cell(curCell.r + offset.y, curCell.c + offset.x);

        if (!lddMap.metadata().is_on_map(destCell)) {
            throw RuntimeError("ldd map is unsound : it has flows out of the map {}", curCell); // throw == pcraster behaviour
        }

        if (!visitCb(curCell, destCell)) {
            return;
        }

        if (lddMap.is_nodata(destCell)) {
            throw RuntimeError("ldd map is unsound : it has flows into NODATA ldd cells {}", curCell); // throw == pcraster behaviour
        }

        dir     = lddMap[destCell];
        curCell = destCell;

        if (traversedCells.find(destCell) != traversedCells.end()) {
            throw RuntimeError("lddMap contains a loop at cell {}", curCell);
        }
    }
}

template <template <typename> typename RasterType, typename IdType>
void add_neighbouring_upstream_cells(const Cell cell, const RasterType<uint8_t>& lddMap, std::optional<IdType> mostDownstreamId, std::vector<std::pair<Cell, std::optional<IdType>>>& cells)
{
    auto& meta = lddMap.metadata();

    auto neighbour = inf::top_left_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 3) {
        cells.emplace_back(neighbour, mostDownstreamId);
    }

    neighbour = inf::top_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 2) {
        cells.emplace_back(neighbour, mostDownstreamId);
    }

    neighbour = inf::top_right_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 1) {
        cells.emplace_back(neighbour, mostDownstreamId);
    }

    neighbour = inf::left_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 6) {
        cells.emplace_back(neighbour, mostDownstreamId);
    }

    neighbour = inf::right_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 4) {
        cells.emplace_back(neighbour, mostDownstreamId);
    }

    neighbour = inf::bottom_left_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 9) {
        cells.emplace_back(neighbour, mostDownstreamId);
    }

    neighbour = inf::bottom_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 8) {
        cells.emplace_back(neighbour, mostDownstreamId);
    }

    neighbour = inf::bottom_right_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 7) {
        cells.emplace_back(neighbour, mostDownstreamId);
    }
}

template <template <typename> typename RasterType, typename VisitCb>
void visit_neighbouring_upstream_cells(const Cell cell, const RasterType<uint8_t>& lddMap, VisitCb&& visitor)
{
    auto& meta = lddMap.metadata();

    auto neighbour = inf::top_left_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 3) {
        visitor(neighbour);
    }

    neighbour = inf::top_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 2) {
        visitor(neighbour);
    }

    neighbour = inf::top_right_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 1) {
        visitor(neighbour);
    }

    neighbour = inf::left_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 6) {
        visitor(neighbour);
    }

    neighbour = inf::right_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 4) {
        visitor(neighbour);
    }

    neighbour = inf::bottom_left_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 9) {
        visitor(neighbour);
    }

    neighbour = inf::bottom_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 8) {
        visitor(neighbour);
    }

    neighbour = inf::bottom_right_cell(cell);
    if (meta.is_on_map(neighbour) && lddMap[neighbour] == 7) {
        visitor(neighbour);
    }
}

template <template <typename> typename RasterType>
bool has_upstream_cells(const Cell cell, const RasterType<uint8_t>& lddMap)
{
    int upstreamCells = 0;

    visit_neighbouring_upstream_cells(cell, lddMap, [&](auto&) {
        ++upstreamCells;
    });

    return upstreamCells > 0;
}

template <template <typename> typename RasterType, typename IdType, typename VisitCb>
void traverse_ldd_upstream(const Cell cell, const RasterType<uint8_t>& lddMap, std::optional<IdType> downstreamId, VisitCb&& visitCb)
{
    assert(!lddMap.is_nodata(cell));

    std::vector<std::pair<Cell, std::optional<IdType>>> upstream;
    add_neighbouring_upstream_cells(cell, lddMap, downstreamId, upstream);
    while (!upstream.empty()) {
        auto [cell, id] = upstream.back();
        id              = visitCb(cell, id);
        upstream.pop_back();
        add_neighbouring_upstream_cells(cell, lddMap, id, upstream);
    }
}

template <template <typename> typename RasterType, typename VisitCb1, typename VisitCb2, typename VisitCb3, typename VisitCb4>
void traverseInvalidLdd(Cell cell, const RasterType<uint8_t>& lddMap,
    VisitCb1&& loopCb,
    VisitCb2&& invalidValueCb,
    VisitCb3&& endsInNodataCb,
    VisitCb4&& outsideOfMapCb)
{
    assert(!lddMap.is_nodata(cell));
    uint8_t dir  = lddMap[cell];
    auto curCell = cell;

    auto incomingCells = 0;
    for (uint8_t i = 1; i <= 9; ++i) {
        if (i == 5) {
            continue;
        }

        auto neighbour = getNeighbour(i, curCell);
        if (lddMap.metadata().is_on_map(neighbour)) {
            if (getDestinationCell(neighbour, lddMap) == curCell) {
                ++incomingCells;
            }
        }
    }

    if (incomingCells > 0) {
        // we are in the middle of a flow, only follow flows from the start
        return;
    }

    std::set<Cell> traversedCells;

    while (dir != 5) {
        traversedCells.insert(curCell);

        if (9 < dir) {
            invalidValueCb(curCell);
            return;
        }

        const Offset& offset = lookupOffsets[dir];
        const Cell destCell(curCell.r + offset.y, curCell.c + offset.x);

        if (!lddMap.metadata().is_on_map(destCell)) {
            outsideOfMapCb(destCell);
            return;
        }

        if (lddMap.is_nodata(destCell)) {
            endsInNodataCb(curCell);
            return;
        }

        if (dir != 5) {
            if (traversedCells.find(destCell) != traversedCells.end()) {
                loopCb(destCell);
                return;
            }
        }

        curCell = destCell;
        dir     = lddMap[destCell];
    }
}

constexpr Offset neighbourLookup[8] = {
    {-1, 1},
    {0, 1},
    {1, 1},
    {-1, 0},
    {1, 0},
    {-1, -1},
    {0, -1},
    {1, -1},
};

template <template <typename> typename RasterType>
double calculateSlopeLength(const int row, const int col,
    RasterType<int32_t>& calcMap,
    const RasterType<uint8_t>& lddMap,
    const RasterType<float>& frictionMap,
    RasterType<float>& result)
{
    if (calcMap(row, col) == 1) {
        return result(row, col);
    }

    if (frictionMap.is_nodata(row, col) || lddMap.is_nodata(row, col)) {
        calcMap(row, col) = 1;
        result.mark_as_nodata(row, col);
        return result(row, col);
    }

    double slopeLength = 0.0;
    bool isNonNan      = false;
    bool isNan         = false;
    auto& meta         = lddMap.metadata();

    for (int i = 0; i < 8; ++i) {
        auto& offset         = detail::neighbourLookup[i];
        const int neighbourR = row + offset.y;
        const int neighbourC = col + offset.x;

        if (!meta.is_on_map(neighbourR, neighbourC)) {
            continue;
        }

        // Check if the neighbour flows into the current cell
        if (lddMap.is_nodata(neighbourR, neighbourC)) {
            continue;
        }

        const auto neighbourDir = lddMap(neighbourR, neighbourC);

        const auto& neighbourOffset = lookupOffsets[neighbourDir];
        const auto neighbourDestR   = neighbourR + neighbourOffset.y;
        const auto neighbourDestC   = neighbourC + neighbourOffset.x;

        if (neighbourDestR == row && neighbourDestC == col) {
            auto neighBourLength = detail::calculateSlopeLength(neighbourR, neighbourC, calcMap, lddMap, frictionMap, result);
            auto distance        = meta.cellSize;
            if (neighbourDir == 1 || neighbourDir == 3 || neighbourDir == 7 || neighbourDir == 9) {
                distance *= sqrt(2.0);
            }

            const auto value = neighBourLength + (distance * (frictionMap(neighbourR, neighbourC) + frictionMap(row, col)) / 2.0);
            if (!isnan(value)) {
                slopeLength = std::max(slopeLength, value);
                isNonNan    = true;
            } else {
                isNan = true;
            }
        }
    }

    if (!isNonNan && isNan) {
        // there was not a single valid value, but there where nan values
        slopeLength = std::numeric_limits<double>::quiet_NaN();
    }

    calcMap(row, col) = 1;
    result(row, col)  = static_cast<float>(slopeLength);
    return slopeLength;
}
}

template <template <typename> typename RasterType>
bool validateLdd(const RasterType<uint8_t>& lddMap,
    std::function<void(int32_t, int32_t)> loopCb,
    std::function<void(int32_t, int32_t)> invalidValueCb,
    std::function<void(int32_t, int32_t)> endsInNodataCb,
    std::function<void(int32_t, int32_t)> outsideOfMapCb)
{
    const int rows = lddMap.rows();
    const int cols = lddMap.cols();
    bool valid     = true;

    std::set<Cell> loops;
    std::mutex mutex;

#pragma omp parallel for shared(loops, mutex)
    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (lddMap.is_nodata(r, c)) {
                continue;
            }

            detail::traverseInvalidLdd(
                Cell(r, c), lddMap,
                [&](Cell cell) {
                    valid = false;
                    if (loopCb) {
                        std::scoped_lock lock(mutex);
                        auto [iter, inserted] = loops.insert(cell);
                        if (inserted) {
                            loopCb(iter->r, iter->c);
                        }
                    }
                },
                [&](Cell cell) {
                    valid = false;
                    if (invalidValueCb) {
                        invalidValueCb(cell.r, cell.c);
                    }
                },
                [&](Cell cell) {
                    valid = false;
                    if (endsInNodataCb) {
                        endsInNodataCb(cell.r, cell.c);
                    }
                },
                [&](Cell cell) {
                    valid = false;
                    if (outsideOfMapCb) {
                        outsideOfMapCb(cell.r, cell.c);
                    }
                });
        }
    }

    return valid;
}

template <template <typename> typename RasterType>
RasterType<uint8_t> fixLdd(const RasterType<uint8_t>& lddMap, std::set<Cell>& errors)
{
    const int rows = lddMap.rows();
    const int cols = lddMap.cols();

    RasterType<uint8_t> result = lddMap.copy();
    auto nodata                = lddMap.metadata().nodata;

    int32_t fixes = 0;

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (lddMap.is_nodata(r, c)) {
                continue;
            }

            detail::traverseInvalidLdd(
                Cell(r, c), result,
                [&](Cell cell) {
                    // ldd contains a loop

                    int32_t inwards = 0;
                    Cell inwardCell;
                    auto cellValue = detail::getNeighbourValue(1, cell, lddMap);
                    if (cellValue > 0 && cellValue == 9) {
                        inwardCell = detail::getNeighbour(1, cell);
                        ++inwards;
                    }

                    cellValue = detail::getNeighbourValue(2, cell, lddMap);
                    if (cellValue > 0 && cellValue == 8) {
                        inwardCell = detail::getNeighbour(2, cell);
                        ++inwards;
                    }

                    cellValue = detail::getNeighbourValue(3, cell, lddMap);
                    if (cellValue > 0 && cellValue == 7) {
                        inwardCell = detail::getNeighbour(3, cell);
                        ++inwards;
                    }

                    cellValue = detail::getNeighbourValue(4, cell, lddMap);
                    if (cellValue > 0 && cellValue == 6) {
                        inwardCell = detail::getNeighbour(4, cell);
                        ++inwards;
                    }

                    cellValue = detail::getNeighbourValue(6, cell, lddMap);
                    if (cellValue > 0 && cellValue == 4) {
                        inwardCell = detail::getNeighbour(6, cell);
                        ++inwards;
                    }

                    cellValue = detail::getNeighbourValue(7, cell, lddMap);
                    if (cellValue > 0 && cellValue == 3) {
                        inwardCell = detail::getNeighbour(7, cell);
                        ++inwards;
                    }

                    cellValue = detail::getNeighbourValue(8, cell, lddMap);
                    if (cellValue > 0 && cellValue == 2) {
                        inwardCell = detail::getNeighbour(8, cell);
                        ++inwards;
                    }

                    cellValue = detail::getNeighbourValue(9, cell, lddMap);
                    if (cellValue > 0 && cellValue == 1) {
                        inwardCell = detail::getNeighbour(9, cell);
                        ++inwards;
                    }

                    if (inwards > 1) {
                        // this cell is not the cause of the loop
                        return;
                    }

                    // find a neighbour of the inward cell, that is also our neighbour
                    assert(inwards == 1);
                    for (uint8_t i = 1; i <= 9; ++i) {
                        if (i == 5) {
                            continue;
                        }

                        auto neighbourNeighbour = detail::getNeighbour(i, inwardCell);
                        if (neighbourNeighbour != cell && detail::cellsAreNeigbours(neighbourNeighbour, cell)) {
                            auto neighbourNeighbourDestCell = detail::getDestinationCell(neighbourNeighbour, lddMap);
                            if (neighbourNeighbourDestCell == inwardCell) {
                                continue;
                            }

                            auto value = lddMap(neighbourNeighbour.r, neighbourNeighbour.c);
                            if (value != 5 && value != 0) {
                                result[cell] = detail::getDirectionToNeighbour(cell, neighbourNeighbour);
                                ++fixes;
                                return;
                            }
                        }
                    }

                    errors.insert(cell);
                },
                [&](Cell cell) {
                    // invalid value in ldd
                    // replace with nodata
                    result[cell] = static_cast<uint8_t>(nodata.value());
                },
                [&](Cell cell) {
                    auto destCell = detail::getDestinationCell(cell, lddMap);
                    // ldd ends in nodata, ignore
                    result[destCell] = 5;
                },
                [&](Cell /*cell*/) {
                    // ldd runs outside of the map
                    // cannot fix this
                    throw RuntimeError("Ldd runs outside of the map, this cannot be fixed");
                });
        }
    }

    return result;
}

template <template <typename> typename RasterType>
RasterType<float> accuflux(const RasterType<uint8_t>& lddMap, const RasterType<float>& freightMap)
{
    if (lddMap.size() != freightMap.size()) {
        throw InvalidArgument("Raster sizes should match");
    }

    const int rows = lddMap.rows();
    const int cols = lddMap.cols();

    RasterType<float> result = freightMap.copy();

    for (int32_t r = 0; r < rows; ++r) {
        for (int32_t c = 0; c < cols; ++c) {
            if (lddMap.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            const auto freight = freightMap(r, c);
            detail::traverse_ldd(Cell(r, c), lddMap, [&result, freight](Cell /*cell*/, Cell destCell) {
                result[destCell] += freight;
                return true;
            });
        }
    }

    return result;
}

template <template <typename> typename RasterType>
RasterType<float> accufractionflux(
    const RasterType<uint8_t>& lddMap,
    const RasterType<float>& freightMap,
    const RasterType<float>& fractionMap)
{
    if (lddMap.size() != freightMap.size() ||
        lddMap.size() != freightMap.size() ||
        lddMap.size() != fractionMap.size()) {
        throw InvalidArgument("Raster sizes should match");
    }

    const int rows = lddMap.rows();
    const int cols = lddMap.cols();
    auto meta      = lddMap.metadata();
    meta.nodata    = std::numeric_limits<float>::quiet_NaN();

    RasterType<float> result(meta);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            result(r, c) = freightMap(r, c) * fractionMap(r, c);
        }
    }

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (lddMap.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            if (freightMap.is_nodata(r, c) || fractionMap.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
            }

            auto freight = static_cast<double>(freightMap(r, c)) * fractionMap(r, c);
            detail::traverse_ldd(Cell(r, c), lddMap, [&](Cell /*cell*/, Cell destCell) {
                freight *= fractionMap[destCell];
                if (std::isnan(freight)) {
                    result.mark_as_nodata(destCell.r, destCell.c);
                } else {
                    result[destCell] += static_cast<float>(freight);
                }
                return true;
            });
        }
    }

    return result;
}

template <template <typename> typename RasterType>
RasterType<float> fluxOrigin(const RasterType<uint8_t>& lddMap,
    const RasterType<float>& freightMap,
    const RasterType<float>& fractionMap,
    const RasterType<int32_t>& stationMap)
{
    if (lddMap.size() != freightMap.size() ||
        lddMap.size() != fractionMap.size() ||
        lddMap.size() != stationMap.size()) {
        throw InvalidArgument("Raster sizes should match");
    }

    auto nan       = std::numeric_limits<double>::quiet_NaN();
    const int rows = lddMap.rows();
    const int cols = lddMap.cols();
    auto meta      = lddMap.metadata();
    meta.nodata    = nan;

    RasterType<float> result(meta, 0.f);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (lddMap.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            double freight = nan;
            if (!freightMap.is_nodata(r, c) && !fractionMap.is_nodata(r, c)) {
                freight = static_cast<double>(freightMap(r, c)) * fractionMap(r, c);
            }

            if (stationMap(r, c) != 0) {
                if (std::isnan(freight)) {
                    result.mark_as_nodata(r, c);
                } else {
                    result(r, c) = static_cast<float>(freight);
                }
                continue;
            }

            detail::traverse_ldd(Cell(r, c), lddMap, [&](Cell /*cell*/, Cell destCell) -> bool {
                if (stationMap[destCell] == 1) {
                    if (std::isnan(freight)) {
                        result.mark_as_nodata(r, c);
                    } else {
                        result(r, c) = static_cast<float>(freight);
                    }

                    return false;
                }

                if (fractionMap.is_nodata(destCell)) {
                    freight = nan;
                } else {
                    freight *= fractionMap[destCell];
                }

                return true;
            });
        }
    }

    return result;
}

template <template <typename> typename RasterType>
RasterType<float> lddCluster(const RasterType<uint8_t>& lddMap, const RasterType<int32_t>& idMap)
{
    if (lddMap.size() != idMap.size()) {
        throw InvalidArgument("Raster sizes should match");
    }

    const int rows = lddMap.rows();
    const int cols = lddMap.cols();
    auto meta      = lddMap.metadata();
    auto nan       = std::numeric_limits<float>::quiet_NaN();

    auto result = raster_cast<float>(idMap);
    result.set_nodata(nan);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (idMap(r, c) != 0) {
                // The current cell is a measurement station, no need to follow the path
                continue;
            }

            if (lddMap.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            detail::traverse_ldd(Cell(r, c), lddMap, [&](Cell /*cell*/, Cell destCell) {
                const int stationId = idMap[destCell];
                if (stationId != 0) {
                    // found station downstream, asign id and abort traversal
                    result(r, c) = static_cast<float>(stationId);
                    result.mark_as_data(r, c);
                    return false;
                }

                return true;
            });
        }
    }

    return result;
}

template <template <typename> typename RasterType>
RasterType<float> lddDist(
    const RasterType<uint8_t>& lddMap,
    const RasterType<float>& pointsMap,
    const RasterType<float>& frictionMap)
{
    if (lddMap.size() != pointsMap.size() ||
        lddMap.size() != frictionMap.size()) {
        throw InvalidArgument("Raster sizes should match");
    }

    auto nan       = std::numeric_limits<float>::quiet_NaN();
    const int rows = lddMap.rows();
    const int cols = lddMap.cols();
    auto meta      = lddMap.metadata();
    meta.nodata    = nan;

    RasterType<float> result(meta, 0.f);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (pointsMap.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            if (lddMap.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            if (pointsMap(r, c) != 0) {
                // The flow ends when a non zero value is in the points map
                continue;
            }

            if (frictionMap.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            double friction         = 0.0;
            bool stoppedInTruePoint = false;

            detail::traverse_ldd(Cell(r, c), lddMap, [&](Cell cell, Cell destCell) -> bool {
                if (pointsMap.is_nodata(destCell)) {
                    // stoppedInTrue is false, so result will get NaN assigned
                    return false;
                }

                const float point = pointsMap[destCell];
                double distance   = meta.cellSize;
                if (cell.r != destCell.r && cell.c != destCell.c) {
                    // diagonal direction
                    distance *= sqrt(2.0);
                }

                friction += distance * (frictionMap[cell] + frictionMap[destCell]) / 2.0;

                if (point != 0) {
                    // traversal stops when encountering a point
                    stoppedInTruePoint = true;
                    return false;
                }

                return true;
            });

            if (stoppedInTruePoint && !std::isnan(friction)) {
                result(r, c) = static_cast<float>(friction);
            } else {
                result.mark_as_nodata(r, c);
            }
        }
    }

    return result;
}

template <template <typename> typename RasterType>
RasterType<float> slopeLength(
    const RasterType<uint8_t>& lddMap,
    const RasterType<float>& frictionMap)
{
    if (lddMap.size() != frictionMap.size()) {
        throw InvalidArgument("Raster sizes should match");
    }

    auto nan       = std::numeric_limits<float>::quiet_NaN();
    const int rows = lddMap.rows();
    const int cols = lddMap.cols();
    auto meta      = lddMap.metadata();
    meta.nodata    = nan;

    RasterType<int32_t> calculatedMap(lddMap.metadata(), 0);
    RasterType<float> result(meta, 0.f);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            detail::calculateSlopeLength(r, c, calculatedMap, lddMap, frictionMap, result);
        }
    }

    return result;
}

template <typename StationIdType, template <typename> typename RasterType>
RasterType<StationIdType> catchment(const RasterType<uint8_t>& lddMap, const RasterType<StationIdType>& stationMap, std::function<void(float)> progressCb = nullptr)
{
    if (lddMap.size() != stationMap.size()) {
        throw InvalidArgument("Raster sizes should match");
    }

    const int rows = lddMap.rows();
    const int cols = lddMap.cols();

    RasterType<StationIdType> result(stationMap.metadata(), 0);

    const float total = inf::truncate<float>(rows * cols);
    std::atomic<int> processed(0);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (progressCb) {
                progressCb(++processed / total);
            }

            if (lddMap.is_nodata(r, c)) {
                result.mark_as_nodata(r, c);
                continue;
            }

            if (lddMap(r, c) != 5) {
                continue;
            }

            std::optional<StationIdType> id;
            if (!stationMap.is_nodata(r, c) && stationMap(r, c) != 0) {
                id = stationMap(r, c);
            }

            detail::traverse_ldd_upstream(Cell(r, c), lddMap, id, [&](Cell cell, std::optional<StationIdType> mostDownStreamId) -> std::optional<StationIdType> {
                if (mostDownStreamId.has_value()) {
                    result[cell] = *mostDownStreamId;
                } else if (stationMap[cell] != 0) {
                    mostDownStreamId = stationMap[cell];
                    result[cell]     = *mostDownStreamId;
                }

                return mostDownStreamId;
            });
        }
    }

    return result;
}

template <template <typename> typename RasterType>
RasterType<float> max_upstream_dist(const RasterType<uint8_t>& ldd, std::function<void(int32_t, int32_t)> progressCb = nullptr)
{
    auto meta = ldd.metadata();
    if (meta.nodata.has_value()) {
        meta.nodata = std::numeric_limits<float>::quiet_NaN();
    }

    const int rows           = ldd.rows();
    const int cols           = ldd.cols();
    const float cellSize     = inf::truncate<float>(meta.cellSize);
    const float cellSizeDiag = cellSize * std::sqrt(2.f);

    RasterType<float> result(meta, 0.f);

    auto total    = rows * cols;
    int processed = 0;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            Cell cell(r, c);

            if (progressCb) {
                progressCb(++processed, total);
            }

            if (ldd.is_nodata(cell)) {
                result.mark_as_nodata(cell);
                continue;
            }

            if (detail::has_upstream_cells(cell, ldd)) {
                continue;
            }

            // traverse the ldd for the most upstream cells
            float totalDistance = 0.f;
            detail::traverse_ldd(cell, ldd, [&](Cell cell, Cell destCell) -> bool {
                if (cell.r != destCell.r && cell.c != destCell.c) {
                    totalDistance += cellSizeDiag;
                } else {
                    totalDistance += cellSize;
                }

                auto& cellValue = result[destCell];
                if (cellValue < totalDistance) {
                    cellValue = totalDistance;
                    return true;
                }

                return false;
            });
        }
    }

    return result;
}
}
