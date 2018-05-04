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

Grid::iterator Grid::begin()
{
    return _cells.begin();
}

Grid::const_iterator Grid::begin() const
{
    return _cells.begin();
}

Grid::const_iterator Grid::cbegin() const
{
    return _cells.cbegin();
}

Grid::iterator Grid::end()
{
    return _cells.end();
}

Grid::const_iterator Grid::end() const
{
    return _cells.end();
}

Grid::const_iterator Grid::cend() const
{
    return _cells.cend();
}

}
