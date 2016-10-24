#pragma once

#include <string>
#include <vector>

namespace OPAQ
{

/** A simple class to represent a rectangular grid cell location
      We will represent an interpolation grid as a collection of cells, much like
      a shapefile is a collection of features.

      \note That we can only represent rectangular gridcells
   */
class Cell
{
public:
    Cell();
    /** Constucts a cell from an ID and a 3D extent, given by an x,y,z range */
    Cell(long ID, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);

    /** Constructs a cell from an ID and a 2D extent, we assume no z information */
    Cell(long ID, double xmin, double xmax, double ymin, double ymax);

    /** Return the Cell ID */
    long getId() const { return id; }

    /** Returns the cell center in x direction */
    double getXc() const { return .5 * (xmax + xmin); }
    /** Returns the cell center in y direction */
    double getYc() const { return .5 * (ymax + ymin); }
    /** Returns the cell center in z direction */
    double getZc() const { return .5 * (zmax + zmin); }

    /** Returns the cell x dimension */
    double getDx() const { return (xmax - xmin); }
    /** Returns the cell y dimension */
    double getDy() const { return (ymax - ymin); }
    /** Returns the cell z dimension */
    double getDz() const { return (zmax - zmin); }

    /** Returns the cell volume
  \note only for 3D cells as it assumes a dz != 0
    */
    double getVolume() const { return getDx() * getDy() * getDz(); }

    /** Returns a cell's surface area, i.e. only surface of it's xy plane */
    double getSurfaceArea() const { return getDx() * getDy(); }

private:
    long id;     // a unique grid cell ID
    double xmin; // lower x value
    double xmax; // upper x value
    double ymin; // lower y value
    double ymax; // upper y value
    double zmin; // lower z value
    double zmax; // upper z value
};

}
