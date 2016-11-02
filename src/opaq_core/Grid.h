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
    void addCell(const Cell& cell);

    size_t cellCount() const noexcept;
    const Cell& cell(size_t index) const;

private:
    std::vector<Cell> _cells;
};

}
