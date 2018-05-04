#include "Point.h"

namespace opaq
{

Point::Point()
: Point(0, 0.0, 0.0, 0.0)
{
}

Point::Point(long id, double x, double y)
: Point(id, x, y, 0.0)
{
}

Point::Point(long id, double x, double y, double z)
: _id(id)
, _x(x)
, _y(y)
, _z(z)
{
}

}
