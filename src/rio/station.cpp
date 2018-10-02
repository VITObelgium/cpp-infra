#include <boost/format.hpp>
#include <sstream>

#include "station.hpp"

namespace rio {

station::station()
: _name(""), _x(0.), _y(0.), _alt(0.), _type(0)
{
}

station::station(std::string name, int type, double x, double y, double alt)
: _name(name), _x(x), _y(y), _alt(alt), _type(type)
{
    std::stringstream ss;
    ss << boost::format("POINT(%.2f %.2f)") % x % y;
    _wkt = ss.str();
}

station::~station()
{
}

std::ostream& operator<<(std::ostream& out, const station& s)
{
    out << s._name
        << ": x=" << s._x
        << ", y=" << s._y
        << ", alt=" << s._alt
        << ", type=" << s._type
        << ", proxy :";
    for (double p : s._proxy)
        out << " " << p;
    out << std::endl;
    return out;
}

}
