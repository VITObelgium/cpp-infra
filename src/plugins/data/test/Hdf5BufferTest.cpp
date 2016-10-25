#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <chrono>

#include "TimeSeries.h"
#include "tools/FileTools.h"
#include "PredictionDatabase.h"

namespace OPAQ
{
namespace Test
{

using namespace date;
using namespace chrono;
using namespace testing;
using namespace chrono_literals;
using namespace std::chrono_literals;

static const std::string s_station = "40AB01";

class Hdf5BufferTest : public testing::Test
{
protected:
    Hdf5BufferTest()
    {
        FileTools::del("test.db");
        //db = std::make_unique<PredictionDatabase>("test.db");
    }

    //std::unique_ptr<PredictionDatabase> db;
};

TEST_F(Hdf5BufferTest, GetSetPrediction)
{
    /*auto basetime = make_date_time(2015_y/jan/1);

    TimeSeries<double> forecast;
    forecast.insert(basetime, 0.5);

    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast);

    EXPECT_DOUBLE_EQ(0.5, db->getPrediction(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d));*/
}

TEST_F(Hdf5BufferTest, GetPredictions)
{
    /*TimeSeries<double> forecast;

    auto basetime = make_date_time(2015_y/jan/1);
    forecast.insert(basetime, 0.5);
    forecast.insert(basetime + 1h, 1.5);
    forecast.insert(basetime + 2h, 2.5);
    forecast.insert(basetime + 3h, 3.5);
    forecast.insert(basetime + 4h, 4.5);

    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast);

    auto results = db->getPredictions("model1", "Ukkel", "pm10", "dayavg", 4_d);

    EXPECT_THAT(results.datetimes(), ContainerEq(std::vector<date_time>{
        basetime,
        basetime + 1h,
        basetime + 2h,
        basetime + 3h,
        basetime + 4h,
    }));
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ 0.5, 1.5, 2.5, 3.5, 4.5 }));*/
}

}
}