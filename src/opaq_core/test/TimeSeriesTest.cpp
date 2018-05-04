#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "TimeSeries.h"
#include "Exceptions.h"

namespace opaq
{
namespace test
{

using namespace date;
using namespace chrono;
using namespace testing;
using namespace std::chrono_literals;

TEST(TimeSeriesTest, Select)
{
    TimeSeries<double> ts;
    ts.reserve(10000);

    auto dt = chrono::make_date_time(2015_y/jan/1);
    for (int i = 0; i < 100000; ++i)
    {
        ts.insert(dt, i);
        dt += 1h * 24;
    }

    auto res = ts.select(chrono::make_date_time(2015_y/aug/1), chrono::make_date_time(2015_y/aug/31));
    EXPECT_EQ(31u, res.size());
}

}
}
