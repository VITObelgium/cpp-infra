#include "infra/interpolate.h"
#include "infra/exception.h"
#include "infra/test/printsupport.h"

#include <doctest/doctest.h>

namespace inf::test {

TEST_CASE("Interpolate")
{
    SUBCASE("To byte range 100")
    {
        CHECK(linear_map_to_byte(0.f, 0.f, 100.f, 1, 4) == 1);
        CHECK(linear_map_to_byte(24.9f, 0.f, 100.f, 1, 4) == 1);
        CHECK(linear_map_to_byte(25.f, 0.f, 100.f, 1, 4) == 2);
        CHECK(linear_map_to_byte(49.9f, 0.f, 100.f, 1, 4) == 2);
        CHECK(linear_map_to_byte(50.f, 0.f, 100.f, 1, 4) == 3);
        CHECK(linear_map_to_byte(74.9f, 0.f, 100.f, 1, 4) == 3);
        CHECK(linear_map_to_byte(75.f, 0.f, 100.f, 1, 4) == 4);
        CHECK(linear_map_to_byte(100.f, 0.f, 100.f, 1, 4) == 4);
    }

    SUBCASE("To byte range")
    {
        CHECK(linear_map_to_byte(1.f, 1.f, 4.f, 1, 4) == 1);
        CHECK(linear_map_to_byte(2.f, 1.f, 4.f, 1, 4) == 2);
        CHECK(linear_map_to_byte(3.f, 1.f, 4.f, 1, 4) == 3);
        CHECK(linear_map_to_byte(4.f, 1.f, 4.f, 1, 4) == 4);
    }

    SUBCASE("Negative range")
    {
        CHECK(linear_map_to_float(-1.2f, -1.2f, 1.2f) == 0.f);
        CHECK(linear_map_to_float(0.f, -1.2f, 1.2f) == 0.5f);
        CHECK(linear_map_to_float(1.2f, -1.2f, 1.2f) == 1.f);
    }
}

}
