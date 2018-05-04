#pragma once

#include <string>
#include <vector>

#include "Cell.h"

namespace opaq
{

/**
     A class to contain the interpolation grid.
     Basically is a container class for a list of gridcells, which can be fetched as a 
     vector by the getCells member function. 
   */
class Grid
{
public:
    using iterator = typename std::vector<Cell>::iterator;
    using const_iterator = typename std::vector<Cell>::const_iterator;

    void addCell(const Cell& cell);

    size_t cellCount() const noexcept;
    const Cell& cell(size_t index) const;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

private:
    std::vector<Cell> _cells;
};

}
