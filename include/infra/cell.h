#pragma once

#include <cinttypes>
#include <limits>
#include <ostream>

namespace inf {

// Represents a point in the raster using r,c coordinates
template <typename T>
struct Cell
{
    Cell() = default;
    Cell(T row, T col)
    : r(row), c(col)
    {
    }

    bool operator==(const Cell<T>& other) const noexcept
    {
        return r == other.r && c == other.c;
    }

    bool operator!=(const Cell<T>& other) const noexcept
    {
        return !(*this == other);
    }

    bool operator<(const Cell<T>& other) const noexcept
    {
        if (r != other.r) {
            return r < other.r;
        } else {
            return c < other.c;
        }
    }

    T r = std::numeric_limits<T>::max();
    T c = std::numeric_limits<T>::max();
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Cell<T>& cell)
{
    return os << cell.r << "x" << cell.c;
}

template <typename T>
Cell<T> leftCell(const Cell<T>& cell) noexcept
{
    return Cell<T>(cell.r, cell.c - 1);
}

template <typename T>
Cell<T> rightCell(const Cell<T>& cell) noexcept
{
    return Cell<T>(cell.r, cell.c + 1);
}

template <typename T>
Cell<T> topCell(const Cell<T>& cell) noexcept
{
    return Cell<T>(cell.r - 1, cell.c);
}

template <typename T>
Cell<T> bottomCell(const Cell<T>& cell) noexcept
{
    return Cell<T>(cell.r + 1, cell.c);
}

template <typename T>
Cell<T> topLeftCell(const Cell<T>& cell) noexcept
{
    return Cell<T>(cell.r - 1, cell.c - 1);
}

template <typename T>
Cell<T> topRightCell(const Cell<T>& cell) noexcept
{
    return Cell<T>(cell.r - 1, cell.c + 1);
}

template <typename T>
Cell<T> bottomLeftCell(const Cell<T>& cell) noexcept
{
    return Cell<T>(cell.r + 1, cell.c - 1);
}

template <typename T>
Cell<T> bottomRightCell(const Cell<T>& cell) noexcept
{
    return Cell<T>(cell.r + 1, cell.c + 1);
}
}
