#include "infra/chrono.h"

#include <doctest/doctest.h>

namespace inf::test {

using namespace date;

TEST_CASE("ChronoTest.time_point_to_utc_string")
{
    chrono::date_point date = 2017_y / feb / 1;

    CHECK("2017-02-01" == inf::chrono::to_string("%Y-%m-%d", date));
}

TEST_CASE("ChronoTest.time_point_to_string_custom_format")
{
    chrono::date_point date = 2017_y / feb / 1;

    CHECK("2017-02-01" == inf::chrono::to_string("%Y-%m-%d", date));
    CHECK("2017-01-02" == inf::chrono::to_string("%Y-%d-%m", date));
}

}
