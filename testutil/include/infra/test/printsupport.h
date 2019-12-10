#pragma once

#include "infra/cell.h"
#include "infra/color.h"
#include "infra/point.h"
#include <optional>
#include <ostream>

namespace inf {
void PrintTo(const Cell& cell, std::ostream* os);
void PrintTo(const Color& c, std::ostream* os);

template <typename T>
void PrintTo(const Point<T>& p, std::ostream* os)
{
    *os << fmt::format("{}", p);
}

}

namespace std {
template <typename T>
void PrintTo(const optional<T>& opt, std::ostream* os)
{
    if (opt.has_value()) {
        *os << opt.value();
    } else {
        *os << "no value";
    }
}
}
