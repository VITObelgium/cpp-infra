#include "gdx/algo/accuflux.h"

namespace gdx::detail {
bool cellsAreNeigbours(Cell c1, Cell c2)
{
    assert(c1 != c2);

    if (std::abs(c1.r - c2.r) <= 1 && std::abs(c1.c - c2.c) <= 1) {
        return true;
    }

    return false;
}

Cell getNeighbour(uint8_t direction, Cell cell)
{
    const Offset& offset = lookupOffsets[direction];
    return Cell(cell.r + offset.y, cell.c + offset.x);
}

uint8_t getDirectionToNeighbour(Cell cell, Cell neighbourCell)
{
    for (int i = 1; i <= 9; ++i) {
        if (i == 5) {
            continue;
        }

        if (getNeighbour(i, cell) == neighbourCell) {
            return i;
        }
    }

    throw std::logic_error("Cells are not neighbours");
}
}
