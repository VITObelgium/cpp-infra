#include "Point.h"

namespace OPAQ
{

Point::Point()
: _id(0)
, _x(0.0)
, _y(0.0)
, _z(0.0)
{
}

Point::Point(long ID, double X, double Y)
: _id(ID)
, _x(X)
, _y(Y)
, _z(0.0)
{
}

Point::Point(long ID, double X, double Y, double Z)
: _id(ID)
, _x(X)
, _y(Y)
, _z(Z)
{
}

}
