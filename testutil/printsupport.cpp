#include "infra/test/printsupport.h"

namespace inf {

std::ostream& operator<<(std::ostream& os, const Cell& cell)
{
    os << fmt::format(fmt::runtime("{}"), cell);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Color& color)
{
    os << fmt::format(fmt::runtime("{}"), color);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Coordinate& coord)
{
    os << fmt::format(fmt::runtime("{}"), coord);
    return os;
}

}
