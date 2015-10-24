/*
 * Location.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#ifndef OPAQ_LOCATION_H_
#define OPAQ_LOCATION_H_

#include <string>
#include <vector>

namespace OPAQ {
    
  /** 
      Class for a point location.
      Holds the x,y,z coordinates of a point location with some setters/getters. 
      Is in OPAQ the parent class for a station
   */
  class Point {
  public:
    /** empty constructor */
    Point();

    /** construct point from an ID for the location and x,y coordinates, no height given (z) 
	\param ID an ID for the current point, could be e.g. station number
	\param X x-coordinate
	\param Y y-coordinate
    */
    Point( long ID, double X, double Y );

    /** construct point from an ID for the location and x,y and z coordinates
	\param ID an ID for the current point, could be e.g. station number
	\param X x-coordinate
	\param Y y-coordinate
	\param Z z-coordinage
    */
    Point( long ID, double X, double Y, double Z );

    virtual ~Point() = 0;
    
    /** return the point ID */
    long   getId() const { return id; }

    /** set the point ID to the given value */
    void   setId(long id) { this->id = id; }

    double getX() const { return x; }
    void   setX(double x) { this->x = x; }
    double getY() const { return y; }
    void   setY(double y) { this->y = y; }
    double getZ() const { return z; }
    void   setZ(double z) { this->z = z; }
    
  private:
    long id;   //!< a unique ID for the point location
    double x;  //!< location x coordinate (in Euclidean system)
    double y;  //!< location y coordinate (in Euclidean system)
    double z;  //!< location altitude above zero plane
  };
  
  
} /* namespace opaq */
#endif /* OPAQ_Point_H_ */
