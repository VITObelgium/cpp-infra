#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <chrono>

#include "tools/FileTools.h"
#include "PredictionDatabase.h"

namespace OPAQ
{
namespace Test
{

using namespace testing;
using namespace std::chrono_literals;

static const std::string s_station = "40AB01";

class SqlBufferTest : public testing::Test
{
protected:
    SqlBufferTest()
    {
        FileTools::del("test.db");
        db = std::make_unique<PredictionDatabase>("test.db");
    }

    std::unique_ptr<PredictionDatabase> db;
};

TEST_F(SqlBufferTest, GetSetPrediction)
{
    auto basetime = DateTime(2015, 1, 1);

    auto now = std::chrono::system_clock::now();
    auto date = std::chrono::system_clock::to_time_t(now);

    db->addPrediction(basetime.getUnixTime(), date, "model1", "Ukkel", "pm10", "dayavg", 4, 0.5);
    
    EXPECT_DOUBLE_EQ(0.5, db->getPrediction(date, "model1", "Ukkel", "pm10", "dayavg", 4));
}

TEST_F(SqlBufferTest, GetPredictions)
{
    auto basetime = DateTime(2015, 1, 1);

    auto now = std::chrono::system_clock::now();
    db->addPrediction(basetime.getUnixTime(), std::chrono::system_clock::to_time_t(now), "model1", "Ukkel", "pm10", "dayavg", 4, 0.5);
    db->addPrediction(basetime.getUnixTime(), std::chrono::system_clock::to_time_t(now + 1h), "model1", "Ukkel", "pm10", "dayavg", 4, 1.5);
    db->addPrediction(basetime.getUnixTime(), std::chrono::system_clock::to_time_t(now + 2h), "model1", "Ukkel", "pm10", "dayavg", 4, 2.5);
    db->addPrediction(basetime.getUnixTime(), std::chrono::system_clock::to_time_t(now + 3h), "model1", "Ukkel", "pm10", "dayavg", 4, 3.5);
    db->addPrediction(basetime.getUnixTime(), std::chrono::system_clock::to_time_t(now + 4h), "model1", "Ukkel", "pm10", "dayavg", 4, 4.5);

    auto startTime = std::chrono::system_clock::to_time_t(now + 1h);
    auto endTime = std::chrono::system_clock::to_time_t(now + 3h);
    auto results = db->getPredictions(startTime, endTime, "model1", "Ukkel", "pm10", "dayavg", 4);

    EXPECT_THAT(results.datetimes(), ContainerEq(std::vector<DateTime>{
        DateTime(std::chrono::system_clock::to_time_t(now + 1h)),
        DateTime(std::chrono::system_clock::to_time_t(now + 2h)),
        DateTime(std::chrono::system_clock::to_time_t(now + 3h))
    }));
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ 1.5, 2.5, 3.5 }));
}

TEST_F(SqlBufferTest, GetInvalidPrediction)
{
    auto now = std::chrono::system_clock::now();
    auto date = std::chrono::system_clock::to_time_t(now);
    EXPECT_THROW(db->getPrediction(date, "model1", "Ukkel", "pm10", "dayavg", 4), RunTimeException);
}

}
}