#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "DateTime.h"

namespace std
{
namespace chrono
{

template<class _Clock, class _Duration = typename _Clock::duration>
ostream& operator<< (ostream& os, const time_point<_Clock, _Duration>& dt)
{
    return os << OPAQ::chrono::to_string(dt);
}

}
}

namespace OPAQ
{
namespace Test
{

using namespace date;
using namespace testing;
using namespace std::chrono_literals;

TEST(DateTimeTest, ToDateString)
{
    EXPECT_EQ(chrono::to_date_string(chrono::make_date_time(2015_y/jan/1)), "2015-01-01");
    EXPECT_EQ(chrono::to_date_string(chrono::make_date_time(2015_y/feb/3)), "2015-02-03");
}

TEST(DateTimeTest, ToString)
{
    EXPECT_EQ(chrono::to_string(chrono::make_date_time(2015_y / jan / 1)), "2015-01-01 00:00:00");
    EXPECT_EQ(chrono::to_string(chrono::make_date_time(2015_y / feb / 3) + 1h + 15min + 10s), "2015-02-03 01:15:10");
}

TEST(DateTimeTest, FromDateString)
{
    EXPECT_EQ(chrono::make_date_time(2015_y/jan/1), chrono::from_date_string("2015-01-01"));
    EXPECT_EQ(chrono::make_date_time(2015_y/feb/3), chrono::from_date_string("2015-02-03"));
}

TEST(DateTimeTest, FromDateTimeString)
{
    EXPECT_EQ(chrono::make_date_time(2015_y/jan/1), chrono::from_date_time_string("2015-01-01 00:00:00"));
    EXPECT_EQ(chrono::make_date_time(2015_y/feb/3) + 1h + 15min + 10s, chrono::from_date_time_string("2015-02-03 01:15:10"));
}

}
}
