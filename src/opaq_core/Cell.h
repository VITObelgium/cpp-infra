#pragma once

#include <string>
#include <vector>

namespace opaq
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
    long getId() const;

    /** Returns the cell center in x direction */
    double getXc() const;
    /** Returns the cell center in y direction */
    double getYc() const;
    /** Returns the cell center in z direction */
    double getZc() const;

    /** Returns the cell x dimension */
    double getDx() const;
    /** Returns the cell y dimension */
    double getDy() const;
    /** Returns the cell z dimension */
    double getDz() const;

    /** Returns the cell volume
     *  \note only for 3D cells as it assumes a dz != 0
     */
    double getVolume() const;

    /** Returns a cell's surface area, i.e. only surface of it's xy plane */
    double getSurfaceArea() const;

    bool operator== (const Cell& other) const noexcept;

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
