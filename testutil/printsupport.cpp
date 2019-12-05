#include "infra/test/printsupport.h"

namespace inf {
void PrintTo(const Cell& cell, std::ostream* os)
{
    *os << fmt::format("{}", cell);
}

void PrintTo(const Color& c, std::ostream* os)
{
    *os << fmt::format("{}", c);
}
}
