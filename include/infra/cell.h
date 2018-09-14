#pragma once

#include <cinttypes>
#include <fmt/core.h>
#include <limits>

namespace inf {

// Represents a point in the raster using r,c coordinates
struct Cell
{
    Cell() = default;
    Cell(int32_t row, int32_t col)
    : r(row)
    , c(col)
    {
    }

    bool operator==(const Cell& other) const noexcept
    {
        return r == other.r && c == other.c;
    }

    bool operator!=(const Cell& other) const noexcept
    {
        return !(*this == other);
    }

    bool operator<(const Cell& other) const noexcept
    {
        if (r != other.r) {
            return r < other.r;
        } else {
            return c < other.c;
        }
    }

    int32_t r = std::numeric_limits<int32_t>::max();
    int32_t c = std::numeric_limits<int32_t>::max();
};

inline Cell leftCell(const Cell& cell) noexcept
{
    return Cell(cell.r, cell.c - 1);
}

inline Cell rightCell(const Cell& cell) noexcept
{
    return Cell(cell.r, cell.c + 1);
}

inline Cell topCell(const Cell& cell) noexcept
{
    return Cell(cell.r - 1, cell.c);
}

inline Cell bottomCell(const Cell& cell) noexcept
{
    return Cell(cell.r + 1, cell.c);
}

inline Cell topLeftCell(const Cell& cell) noexcept
{
    return Cell(cell.r - 1, cell.c - 1);
}

inline Cell topRightCell(const Cell& cell) noexcept
{
    return Cell(cell.r - 1, cell.c + 1);
}

inline Cell bottomLeftCell(const Cell& cell) noexcept
{
    return Cell(cell.r + 1, cell.c - 1);
}

inline Cell bottomRightCell(const Cell& cell) noexcept
{
    return Cell(cell.r + 1, cell.c + 1);
}
}

namespace fmt {
template <>
struct formatter<inf::Cell>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const inf::Cell& cell, FormatContext& ctx)
    {
        return format_to(ctx.begin(), "[{}, {}]", cell.r, cell.c);
    }
};
}
