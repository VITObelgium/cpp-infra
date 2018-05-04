#pragma once

#include <string>
#include <vector>

namespace opaq
{

/**
      Class for a point location.
      Holds the x,y,z coordinates of a point location with some setters/getters.
      Is in OPAQ the parent class for a station
   */
class Point
{
public:
    Point();

    /** construct point from an ID for the location and x,y coordinates, no height given (z)
  \param ID an ID for the current point, could be e.g. station number
  \param X x-coordinate
  \param Y y-coordinate
    */
    Point(long ID, double X, double Y);

    virtual ~Point() = default;

    /** construct point from an ID for the location and x,y and z coordinates
  \param ID an ID for the current point, could be e.g. station number
  \param X x-coordinate
  \param Y y-coordinate
  \param Z z-coordinage
    */
    Point(long ID, double X, double Y, double Z);

    /** return the point ID */
    long getId() const { return _id; }

    /** set the point ID to the given value */
    void setId(long id) { _id = id; }

    double getX() const { return _x; }
    void setX(double x) { _x = x; }
    double getY() const { return _y; }
    void setY(double y) { _y = y; }
    double getZ() const { return _z; }
    void setZ(double z) { _z = z; }

private:
    long _id;  //!< a unique ID for the point location
    double _x; //!< location x coordinate (in Euclidean system)
    double _y; //!< location y coordinate (in Euclidean system)
    double _z; //!< location altitude above zero plane
};

}
