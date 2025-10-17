#include "infra/chrono.h"

#include <doctest/doctest.h>

namespace inf::test {

using namespace std::chrono_literals;

TEST_CASE("ChronoTest.time_point_to_utc_string")
{
    chrono::date_point date = std::chrono::sys_days(2017y / std::chrono::February / 1);

    CHECK("2017-02-01" == inf::chrono::to_string("%Y-%m-%d", date));
}

TEST_CASE("ChronoTest.time_point_to_utc_string_ambiguous")
{
    auto tp = chrono::local_time_point_from_string("2019-10-27 02:20:00.000", "%Y-%m-%d %H:%M:%S");
    REQUIRE(tp.has_value());

    {
        auto utcTime = chrono::localtime_to_utc(*tp, chrono::choose::latest);
        REQUIRE(utcTime.has_value());

        chrono::time_point expected = std::chrono::sys_days(2019y / std::chrono::October / 27) + 1h;
        CHECK(expected == *utcTime);
    }

    {
        auto utcTime = chrono::localtime_to_utc(*tp, chrono::choose::earliest);
        REQUIRE(utcTime.has_value());

        chrono::time_point expected = std::chrono::sys_days(2019y / std::chrono::October / 27) + 0h;
        CHECK(expected == *utcTime);
    }
}

TEST_CASE("ChronoTest.time_point_to_string_custom_format")
{
    chrono::date_point date = std::chrono::sys_days(2017y / std::chrono::February / 1);

    CHECK("2017-02-01" == inf::chrono::to_string("%Y-%m-%d", date));
    CHECK("2017-01-02" == inf::chrono::to_string("%Y-%d-%m", date));
}

}
