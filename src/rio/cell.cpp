#include "cell.hpp"

#include <fmt/format.h>
#include <sstream>

namespace rio {

cell::cell(size_t fid, double cx, double cy, double size, double alt)
: _id(fid)
, _cx(cx)
, _cy(cy)
, _size(size)
, _alt(alt)
{
    // generate wkt
    double xmin = _cx - .5 * _size;
    double xmax = _cx + .5 * _size;
    double ymin = _cy - .5 * _size;
    double ymax = _cy + .5 * _size;

    _wkt = fmt::format("POLYGON(({.2f} {.2f}, {.2f} {.2f}, {.2f} {.2f}, {.2f} {.2f}, {.2f} {.2f} ))", xmin, ymin, xmax, ymin, xmax, ymax, xmin, ymax, xmin, ymin);
}

cell::~cell()
{
}

std::ostream& operator<<(std::ostream& out, const cell& c)
{
    out << "id=" << c._id
        << ", wkt=" << c._wkt
        << ", alt=" << c._alt
        << ", proxy :";
    for (double p : c._proxy)
        out << " " << p;
    out << std::endl;
    return out;
}

}
