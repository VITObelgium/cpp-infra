#include "infra/interpolate.h"
#include "infra/exception.h"
#include "infra/test/printsupport.h"

#include <doctest/doctest.h>

namespace inf::test {

TEST_CASE("Interpolate")
{
    SUBCASE("Negative range")
    {
        CHECK(linear_map_to_float(-1.2f, -1.2f, 1.2f) == 0.f);
        CHECK(linear_map_to_float(0.f, -1.2f, 1.2f) == 0.5f);
        CHECK(linear_map_to_float(1.2f, -1.2f, 1.2f) == 1.f);
    }
}

}
