#pragma once

#include "gdx/cell.h"
#include "gdx/exception.h"
#include "gdx/log.h"

#include <cassert>
#include <gsl/span>
#include <limits>
#include <vector>

namespace gdx {

static const uint8_t s_markTodo(0);
static const uint8_t s_markBorder(1);
static const uint8_t s_markDone(2);

enum class ClusterDiagonals
{
    Include,
    Exclude
};

template <typename T>
class FiLo
{
public:
    FiLo(int nRows, int nCols)
    : head(0)
    , tail(0)
    , filo(nRows * nCols)
    {
    }

    void reserve_space(int nRows, int nCols)
    {
        filo.resize(nRows * nCols);
    }

    void clear()
    {
        head = tail = 0;
    }

    int size() const noexcept
    {
        return (tail + filo.size() - head) % int(filo.size());
    }

    bool empty() const noexcept
    {
        return tail == head;
    }

    void push_back(T value)
    {
        filo[tail] = value;
        tail       = (tail + 1) % filo.size();

        if (tail == head) {
            throw RuntimeError("FiLo overflow");
        }
    }

    T pop_head()
    {
        assert(!empty());
        auto ret = filo[head];
        head     = (head + 1) % filo.size();
        return ret;
    }

private:
    int head, tail;
    std::vector<T> filo;
};

template <template <typename> typename RasterType>
inline void insertCell(const Cell& cell, std::vector<Cell>& clusterCells, RasterType<uint8_t>& mark, FiLo<Cell>& border)
{
    mark(cell.r, cell.c) = s_markBorder;
    border.push_back(cell);
    clusterCells.push_back(cell);
}

template <template <typename> typename RasterType>
inline void insertCell(const Cell& cell, RasterType<uint8_t>& mark, FiLo<Cell>& border)
{
    mark(cell.r, cell.c) = s_markBorder;
    border.push_back(cell);
}

template <template <typename> typename RasterType, typename T>
void handleCell(const Cell cell,
    const T clusterValue, std::vector<Cell>& clusterCells,
    RasterType<uint8_t>& mark,
    FiLo<Cell>& border, const RasterType<T>& raster)
{
    if (raster.is_nodata(cell)) {
        return;
    }

    if (raster[cell] == clusterValue && mark[cell] == s_markTodo) {
        insertCell(cell, clusterCells, mark, border);
    }
}

template <typename Callable>
void visitNeighbourCells(Cell cell, int32_t rows, int32_t cols, Callable&& callable)
{
    bool isLeftBorder  = cell.c == 0;
    bool isRightBorder = cell.c == cols - 1;

    bool isTopBorder    = cell.r == 0;
    bool isBottomBorder = cell.r == rows - 1;

    if (!isRightBorder) {
        callable(right_cell(cell));
    }

    if (!isLeftBorder) {
        callable(left_cell(cell));
    }

    if (!isBottomBorder) {
        callable(bottom_cell(cell));
    }

    if (!isTopBorder) {
        callable(top_cell(cell));
    }
}

template <typename Callable>
void visitNeighbourDiagCells(Cell cell, int32_t rows, int32_t cols, Callable&& callable)
{
    bool isLeftBorder  = cell.c == 0;
    bool isRightBorder = cell.c == cols - 1;

    bool isTopBorder    = cell.r == 0;
    bool isBottomBorder = cell.r == rows - 1;

    if (!(isBottomBorder || isRightBorder)) {
        callable(bottom_right_cell(cell));
    }

    if (!(isTopBorder || isRightBorder)) {
        callable(top_right_cell(cell));
    }

    if (!(isBottomBorder || isLeftBorder)) {
        callable(bottom_left_cell(cell));
    }

    if (!(isTopBorder || isLeftBorder)) {
        callable(top_left_cell(cell));
    }
}

template <typename RasterType>
void showWarningIfClusteringOnFloats(const RasterType&)
{
    if constexpr (std::is_floating_point_v<typename RasterType::value_type>) {
        Log::warn("Performing cluster operation on floating point raster");
    }
}
}
