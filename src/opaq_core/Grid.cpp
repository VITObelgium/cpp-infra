#include "Grid.h"

namespace OPAQ
{

Grid::Grid()
{
}

void Grid::addCell(const Cell& cell)
{
    _cells.push_back(cell);
}

size_t Grid::cellCount() const noexcept
{
    return _cells.size();
}

}
