#pragma once

#include "infra/cell.h"
#include "infra/color.h"
#include "infra/coordinate.h"
#include "infra/point.h"

#include <optional>
#include <ostream>

namespace inf {

std::ostream& operator<<(std::ostream& os, const Cell& cell);
std::ostream& operator<<(std::ostream& os, const Color& cell);
std::ostream& operator<<(std::ostream& os, const Coordinate& cell);

template <typename T>
std::ostream& operator<<(std::ostream& os, const Point<T>& p)
{
    os << fmt::format("{}", p);
    return os;
}

}

namespace std {
template <typename T>
std::ostream& operator<<(std::ostream& os, const optional<T>& opt)
{
    if (opt.has_value()) {
        os << fmt::format("{}", opt.value());
    } else {
        os << "no value";
    }

    return os;
}
}
