#include "Grid.h"

namespace opaq
{

void Grid::addCell(const Cell& cell)
{
    _cells.push_back(cell);
}

size_t Grid::cellCount() const noexcept
{
    return _cells.size();
}

const Cell& Grid::cell(size_t index) const
{
    return _cells.at(index);
}

}
