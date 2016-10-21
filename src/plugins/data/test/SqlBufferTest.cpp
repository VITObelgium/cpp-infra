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
    auto basetime = make_date_time(2015_y/jan/1);

    TimeSeries<double> forecast;
    forecast.insert(basetime, 0.5);

    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast);

    EXPECT_DOUBLE_EQ(0.5, db->getPrediction(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d));
}

TEST_F(SqlBufferTest, GetPredictions)
{
    TimeSeries<double> forecast;

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
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ 0.5, 1.5, 2.5, 3.5, 4.5 }));
}

TEST_F(SqlBufferTest, DuplicatePrediction)
{
    TimeSeries<double> forecast;

    auto basetime = make_date_time(2015_y/jan/1);
    forecast.insert(basetime, 0.5);
    forecast.insert(basetime, 1.5);

    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast);

    auto results = db->getPredictionValues(basetime, "Ukkel", "pm10", "dayavg", 4_d);
    EXPECT_THAT(results, ContainerEq(std::vector<double>{ 1.5 }));
}

TEST_F(SqlBufferTest, DuplicatePredictionMultipleTransactions)
{
    auto basetime = make_date_time(2015_y/jan/1);

    TimeSeries<double> forecast1, forecast2;
    forecast1.insert(basetime, 0.5);
    forecast2.insert(basetime, 1.5);

    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast1);
    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast2);

    auto results = db->getPredictionValues(basetime, "Ukkel", "pm10", "dayavg", 4_d);
    EXPECT_THAT(results, ContainerEq(std::vector<double>{ 1.5 }));
}

TEST_F(SqlBufferTest, GetPredictionValues)
{
    auto basetime = make_date_time(2015_y/jan/1);

    TimeSeries<double> model1Forecast;
    model1Forecast.insert(basetime, 0.5);
    model1Forecast.insert(basetime + 1h, 2.5);
    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, model1Forecast);

    TimeSeries<double> model2Forecast;
    model2Forecast.insert(basetime, 1.5);
    model2Forecast.insert(basetime + 1h, 3.5);
    db->addPredictions(basetime, "model2", "Ukkel", "pm10", "dayavg", 4_d, model2Forecast);

    TimeSeries<double> model1ForecastPm25;
    model1ForecastPm25.insert(basetime, 10.5);
    model1ForecastPm25.insert(basetime + 1h, 12.5);
    db->addPredictions(basetime, "model1", "Ukkel", "pm25", "dayavg", 4_d, model1ForecastPm25);

    TimeSeries<double> model2ForecastPm25;
    model2ForecastPm25.insert(basetime, 11.5);
    model2ForecastPm25.insert(basetime + 1h, 13.5);
    db->addPredictions(basetime, "model2", "Ukkel", "pm25", "dayavg", 4_d, model2ForecastPm25);

    EXPECT_THAT(db->getPredictionValues(basetime, "Ukkel", "pm10", "dayavg", 4_d), ContainerEq(std::vector<double>{ 0.5, 1.5 }));
    EXPECT_THAT(db->getPredictionValues(basetime + 1h, "Ukkel", "pm25", "dayavg", 4_d), ContainerEq(std::vector<double>{ 12.5, 13.5 }));
}

TEST_F(SqlBufferTest, GetModelNames)
{
    auto basetime = make_date_time(2015_y/jan/1);

    TimeSeries<double> forecast1, forecast2;
    forecast1.insert(basetime, 1.5);
    forecast2.insert(basetime + 1h, 1.5);

    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast1);
    db->addPredictions(basetime, "model2", "Ukkel", "pm10", "dayavg", 4_d, forecast1);
    db->addPredictions(basetime, "model2", "Ukkel", "pm10", "dayavg", 4_d, forecast2);
    db->addPredictions(basetime, "model3", "Ukkel", "pm25", "dayavg", 4_d, forecast1);
    db->addPredictions(basetime, "model3", "Ukkel", "pm25", "dayavg", 4_d, forecast2);
    db->addPredictions(basetime, "model3", "Ukkel", "pm25", "8h", 4_d, forecast2);

    EXPECT_THAT(db->getModelNames("pm10", "dayavg"), ContainerEq(std::vector<std::string>{ "model1", "model2" }));
    EXPECT_THAT(db->getModelNames("pm25", "8h"), ContainerEq(std::vector<std::string>{ "model3" }));
    EXPECT_TRUE(db->getModelNames("pm25", "9h").empty());
}

TEST_F(SqlBufferTest, GetPredictionsRange)
{
    auto basetime = make_date_time(2015_y/jan/1);
    TimeSeries<double> forecast;
    forecast.insert(basetime, 0.5);
    forecast.insert(basetime + 1h, 1.5);
    forecast.insert(basetime + 2h, 2.5);
    forecast.insert(basetime + 3h, 3.5);
    forecast.insert(basetime + 4h, 4.5);

    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast);

    auto startTime = basetime + 1h;
    auto endTime = basetime + 3h;
    auto results = db->getPredictions(startTime, endTime, "model1", "Ukkel", "pm10", "dayavg", 4_d);

    EXPECT_THAT(results.datetimes(), ContainerEq(std::vector<chrono::date_time>{
        basetime + 1h,
        basetime + 2h,
        basetime + 3h
    }));
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ 1.5, 2.5, 3.5 }));
}

TEST_F(SqlBufferTest, GetInvalidPrediction)
{
    auto now = std::chrono::system_clock::now();
    EXPECT_THROW(db->getPrediction(now, "model1", "Ukkel", "pm10", "dayavg", 4_d), RunTimeException);
}

}
}