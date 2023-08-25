#pragma once

#include <cinttypes>
#include <cmath>
#include <fmt/core.h>
#include <limits>

namespace inf {

// Represents a point in the raster using r,c coordinates
struct Cell
{
    constexpr Cell() noexcept = default;
    constexpr Cell(int32_t row, int32_t col)
    : r(row)
    , c(col)
    {
    }

    constexpr bool operator==(const Cell& other) const noexcept
    {
        return r == other.r && c == other.c;
    }

    constexpr bool operator!=(const Cell& other) const noexcept
    {
        return !(*this == other);
    }

    constexpr bool operator<(const Cell& other) const noexcept
    {
        if (r != other.r) {
            return r < other.r;
        }

        return c < other.c;
    }

    constexpr bool is_valid() const
    {
        return r >= 0 && c >= 0;
    }

    int32_t r = -1;
    int32_t c = -1;
};

inline Cell left_cell(const Cell& cell) noexcept
{
    return Cell(cell.r, cell.c - 1);
}

inline Cell right_cell(const Cell& cell) noexcept
{
    return Cell(cell.r, cell.c + 1);
}

inline Cell top_cell(const Cell& cell) noexcept
{
    return Cell(cell.r - 1, cell.c);
}

inline Cell bottom_cell(const Cell& cell) noexcept
{
    return Cell(cell.r + 1, cell.c);
}

inline Cell top_left_cell(const Cell& cell) noexcept
{
    return Cell(cell.r - 1, cell.c - 1);
}

inline Cell top_right_cell(const Cell& cell) noexcept
{
    return Cell(cell.r - 1, cell.c + 1);
}

inline Cell bottom_left_cell(const Cell& cell) noexcept
{
    return Cell(cell.r + 1, cell.c - 1);
}

inline Cell bottom_right_cell(const Cell& cell) noexcept
{
    return Cell(cell.r + 1, cell.c + 1);
}

inline void increment_cell(Cell& cell, int32_t cols)
{
    ++cell.c;
    if (cell.c >= cols) {
        cell.c = 0;
        ++cell.r;
    }
}

inline double distance(const Cell& lhs, const Cell& rhs)
{
    auto x = rhs.c - lhs.c;
    auto y = rhs.r - lhs.r;

    return std::sqrt((x * x) + (y * y));
}

}

namespace std {
template <>
struct hash<inf::Cell>
{
    size_t operator()(const inf::Cell& cell) const
    {
        int64_t h1 = size_t(cell.r) << 32;
        int64_t h2 = cell.c;
        return hash<long long>()(h1 | h2);
    }
};
}

template <>
struct fmt::formatter<inf::Cell>
{
    FMT_CONSTEXPR20 auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    auto format(const inf::Cell& cell, format_context& ctx) const -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "[{}, {}]", cell.r, cell.c);
    }
};
