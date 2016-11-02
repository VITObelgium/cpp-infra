#include "Cell.h"

namespace opaq
{

Cell::Cell()
: id(0)
, xmin(0.0)
, xmax(0.0)
, ymin(0.0)
, ymax(0.0)
, zmin(0.0)
, zmax(0.0)
{
}

Cell::Cell(long ID, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax)
: id(ID)
, xmin(xmin)
, xmax(xmax)
, ymin(ymin)
, ymax(ymax)
, zmin(zmin)
, zmax(zmax)
{
}

Cell::Cell(long ID, double xmin, double xmax, double ymin, double ymax)
: id(ID)
, xmin(xmin)
, xmax(xmax)
, ymin(ymin)
, ymax(ymax)
, zmin(0.)
, zmax(0.)
{
}

bool Cell::operator==(const Cell& other) const noexcept
{
    return id == other.id &&
           xmin == other.xmin &&
           xmax == other.xmax &&
           ymin == other.ymin &&
           ymax == other.ymax &&
           zmin == other.zmin &&
           zmax == other.zmax;
}

}
