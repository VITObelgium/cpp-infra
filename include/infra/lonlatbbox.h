#pragma once

#include "infra/coordinate.h"

#include <cassert>
#include <cstdint>

namespace inf {

struct LonLatBBox
{
    inf::Coordinate topLeft;
    inf::Coordinate bottomRight;

    bool is_valid() const noexcept
    {
        return topLeft.is_valid() && bottomRight.is_valid();
    }
};

}
