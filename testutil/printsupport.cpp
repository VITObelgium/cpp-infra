#include "infra/test/printsupport.h"

namespace inf {

std::ostream& operator<<(std::ostream& os, const Cell& cell)
{
    os << fmt::format("{}", cell);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Color& color)
{
    os << fmt::format("{}", color);
    return os;
}

}
