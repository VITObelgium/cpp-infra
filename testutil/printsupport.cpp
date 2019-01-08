#include "gdx/test/printsupport.h"

namespace inf {
void PrintTo(const Cell& cell, std::ostream* os)
{
    *os << fmt::format("{}", cell);
}
}
