#pragma once

#include "gdx/maskedraster.h"
#include "infra/cell.h"

namespace inf {
void PrintTo(const Cell& cell, std::ostream* os);
}

namespace gdx {

template <typename T>
void PrintTo(const MaskedRaster<T>& ras, std::ostream* os)
{
    *os << ras.metadata().to_string() << "\n"
        << ras.eigen_const_data();
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
