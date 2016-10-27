#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "DateTime.h"
#include "Exceptions.h"

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
using namespace chrono;
using namespace testing;
using namespace std::chrono_literals;

TEST(DateTimeTest, DefaultDateToString)
{
    EXPECT_EQ("1970-01-01 00:00:00", chrono::to_string(chrono::date_time()));
}

TEST(DateTimeTest, DefaultDateToDateString)
{
    EXPECT_EQ("1970-01-01", chrono::to_date_string(chrono::date_time()));
}

TEST(DateTimeTest, NegativeDateToString)
{
    EXPECT_EQ("1969-12-31 00:00:00", chrono::to_string(chrono::date_time() - days(1)));
    EXPECT_EQ("1969-12-31 23:00:00", chrono::to_string(chrono::date_time() - 1h));
}

TEST(DateTimeTest, NegativeDateToDateString)
{
    EXPECT_EQ("1969-12-31", chrono::to_date_string(chrono::date_time() - days(1)));
    EXPECT_EQ("1969-12-31", chrono::to_date_string(chrono::date_time() - 1h));
}

TEST(DateTimeTest, ToDateString)
{
    EXPECT_EQ("2015-01-01", chrono::to_date_string(chrono::make_date_time(2015_y/jan/1)));
    EXPECT_EQ("2015-02-03", chrono::to_date_string(chrono::make_date_time(2015_y/feb/3)));
}

TEST(DateTimeTest, ToString)
{
    EXPECT_EQ("2015-01-01 00:00:00", chrono::to_string(chrono::make_date_time(2015_y / jan / 1)));
    EXPECT_EQ("2015-02-03 01:15:10", chrono::to_string(chrono::make_date_time(2015_y / feb / 3) + 1h + 15min + 10s));
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
