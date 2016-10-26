#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <chrono>

#include "../SqlBuffer.h"
#include "../Hdf5Buffer.h"

#include "config.h"
#include "testconfig.h"

#include "Engine.h"
#include "TimeSeries.h"
#include "ComponentManagerFactory.h"
#include "tools/FileTools.h"
#include "PredictionDatabase.h"

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
namespace test
{

using namespace date;
using namespace chrono;
using namespace testing;
using namespace chrono_literals;
using namespace std::string_literals;
using namespace std::chrono_literals;

class EngineMock : public IEngine
{
public:
    MOCK_METHOD0(pollutantManager, Config::PollutantManager&());
    MOCK_METHOD0(componentManager, ComponentManager&());
};

template <typename BufferType>
class ForecastBufferTest : public Test
{
protected:
    ForecastBufferTest()
    : _buffer(&_concreteBuffer)
    {
        FileTools::del("test.db");

        auto configXml =
            "<config>"
            "    <filename>test.db</filename>"
            "    <basetime_resolution>24</basetime_resolution>"
            "    <fctime_resolution>24</fctime_resolution>"
            "</config>"s;

        TiXmlDocument doc;
        doc.Parse(configXml.c_str(), 0, TIXML_ENCODING_UTF8);
        auto* config = doc.FirstChildElement("config");

        auto name = BufferType::name();

        _buffer->configure(config, name, _engineMock);
    }

    EngineMock _engineMock;
    BufferType _concreteBuffer;
    ForecastBuffer* _buffer;
};

using BufferTypes = testing::Types<Hdf5Buffer, SqlBuffer>;
TYPED_TEST_CASE(ForecastBufferTest, BufferTypes);

TYPED_TEST(ForecastBufferTest, GetTimeResolution)
{
    EXPECT_EQ(24h, this->_buffer->getTimeResolution());
}

TYPED_TEST(ForecastBufferTest, GetBaseTimeResolution)
{
    EXPECT_EQ(24h, this->_buffer->getBaseTimeResolution());
}

TYPED_TEST(ForecastBufferTest, GetModelNamesEmpty)
{
    // no data in the buffer
    EXPECT_THROW(this->_buffer->getModelNames("pm10", Aggregation::DayAvg).empty(), NotAvailableException);
}

TYPED_TEST(ForecastBufferTest, GetModelNames)
{
    auto basetime = make_date_time(2015_y/jan/1);
    TimeSeries<double> forecast1, forecast2;
    forecast1.insert(basetime, 1.5);
    forecast2.insert(basetime + 24h, 2.5);

    this->_buffer->setCurrentModel("model1");
    this->_buffer->setValues(basetime, forecast1, "Ukkel", "pm10", Aggregation::DayAvg);

    this->_buffer->setCurrentModel("model2");
    this->_buffer->setValues(basetime, forecast1, "Ukkel", "pm10", Aggregation::DayAvg);
    this->_buffer->setValues(basetime, forecast2, "Ukkel", "pm10", Aggregation::DayAvg);

    this->_buffer->setCurrentModel("model3");
    this->_buffer->setValues(basetime, forecast1, "Ukkel", "pm25", Aggregation::DayAvg);
    this->_buffer->setValues(basetime, forecast2, "Ukkel", "pm25", Aggregation::DayAvg);
    this->_buffer->setValues(basetime, forecast2, "Ukkel", "pm25", Aggregation::Max8h);

    EXPECT_THAT(this->_buffer->getModelNames("pm10", Aggregation::DayAvg), ContainerEq(std::vector<std::string>{ "model1", "model2" }));
    EXPECT_THAT(this->_buffer->getModelNames("pm25", Aggregation::Max8h), ContainerEq(std::vector<std::string>{ "model3" }));
    EXPECT_THROW(this->_buffer->getModelNames("pm25", Aggregation::Max1h), NotAvailableException);
}

TYPED_TEST(ForecastBufferTest, SetGetForecastValues)
{
    auto basetime1 = make_date_time(2015_y / jan / 1);
    auto basetime2 = make_date_time(2015_y / jan / 2);

    auto fcHor = 1_d;

    TimeSeries<double> forecast1, forecast2;
    forecast1.insert(basetime1 + fcHor, 1.5);
    forecast2.insert(basetime2 + fcHor, 2.5);

    this->_buffer->setCurrentModel("model1");
    this->_buffer->setValues(basetime1, forecast1, "Ukkel", "pm10", Aggregation::DayAvg);
    this->_buffer->setValues(basetime2, forecast2, "Ukkel", "pm10", Aggregation::DayAvg);

    auto results = this->_buffer->getForecastValues(fcHor, basetime1 + fcHor, basetime2 + fcHor, "Ukkel", "pm10", Aggregation::DayAvg);

    EXPECT_THAT(results.datetimes(), ContainerEq(std::vector<chrono::date_time>{
        basetime1 + fcHor,
        basetime2 + fcHor,
    }));
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ 1.5, 2.5 }));
}

//TEST_F(SqlBufferTest, DuplicatePrediction)
//{
//    TimeSeries<double> forecast;
//
//    auto basetime = make_date_time(2015_y/jan/1);
//    forecast.insert(basetime, 0.5);
//    forecast.insert(basetime, 1.5);
//
//    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast);
//
//    auto results = db->getPredictionValues(basetime, "Ukkel", "pm10", "dayavg", 4_d);
//    EXPECT_THAT(results, ContainerEq(std::vector<double>{ 1.5 }));
//}
//
//TEST_F(SqlBufferTest, DuplicatePredictionMultipleTransactions)
//{
//    auto basetime = make_date_time(2015_y/jan/1);
//
//    TimeSeries<double> forecast1, forecast2;
//    forecast1.insert(basetime, 0.5);
//    forecast2.insert(basetime, 1.5);
//
//    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast1);
//    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast2);
//
//    auto results = db->getPredictionValues(basetime, "Ukkel", "pm10", "dayavg", 4_d);
//    EXPECT_THAT(results, ContainerEq(std::vector<double>{ 1.5 }));
//}
//
//TEST_F(SqlBufferTest, GetPredictionValues)
//{
//    auto basetime = make_date_time(2015_y/jan/1);
//
//    TimeSeries<double> model1Forecast;
//    model1Forecast.insert(basetime, 0.5);
//    model1Forecast.insert(basetime + 1h, 2.5);
//    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, model1Forecast);
//
//    TimeSeries<double> model2Forecast;
//    model2Forecast.insert(basetime, 1.5);
//    model2Forecast.insert(basetime + 1h, 3.5);
//    db->addPredictions(basetime, "model2", "Ukkel", "pm10", "dayavg", 4_d, model2Forecast);
//
//    TimeSeries<double> model1ForecastPm25;
//    model1ForecastPm25.insert(basetime, 10.5);
//    model1ForecastPm25.insert(basetime + 1h, 12.5);
//    db->addPredictions(basetime, "model1", "Ukkel", "pm25", "dayavg", 4_d, model1ForecastPm25);
//
//    TimeSeries<double> model2ForecastPm25;
//    model2ForecastPm25.insert(basetime, 11.5);
//    model2ForecastPm25.insert(basetime + 1h, 13.5);
//    db->addPredictions(basetime, "model2", "Ukkel", "pm25", "dayavg", 4_d, model2ForecastPm25);
//
//    EXPECT_THAT(db->getPredictionValues(basetime, "Ukkel", "pm10", "dayavg", 4_d), ContainerEq(std::vector<double>{ 0.5, 1.5 }));
//    EXPECT_THAT(db->getPredictionValues(basetime + 1h, "Ukkel", "pm25", "dayavg", 4_d), ContainerEq(std::vector<double>{ 12.5, 13.5 }));
//}
//
//TEST_F(SqlBufferTest, GetModelNames)
//{
//    auto basetime = make_date_time(2015_y/jan/1);
//
//    TimeSeries<double> forecast1, forecast2;
//    forecast1.insert(basetime, 1.5);
//    forecast2.insert(basetime + 1h, 1.5);
//
//    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast1);
//    db->addPredictions(basetime, "model2", "Ukkel", "pm10", "dayavg", 4_d, forecast1);
//    db->addPredictions(basetime, "model2", "Ukkel", "pm10", "dayavg", 4_d, forecast2);
//    db->addPredictions(basetime, "model3", "Ukkel", "pm25", "dayavg", 4_d, forecast1);
//    db->addPredictions(basetime, "model3", "Ukkel", "pm25", "dayavg", 4_d, forecast2);
//    db->addPredictions(basetime, "model3", "Ukkel", "pm25", "8h", 4_d, forecast2);
//
//    EXPECT_THAT(db->getModelNames("pm10", "dayavg"), ContainerEq(std::vector<std::string>{ "model1", "model2" }));
//    EXPECT_THAT(db->getModelNames("pm25", "8h"), ContainerEq(std::vector<std::string>{ "model3" }));
//    EXPECT_TRUE(db->getModelNames("pm25", "9h").empty());
//}
//
//TEST_F(SqlBufferTest, GetPredictionsRange)
//{
//    auto basetime = make_date_time(2015_y/jan/1);
//    TimeSeries<double> forecast;
//    forecast.insert(basetime, 0.5);
//    forecast.insert(basetime + 1h, 1.5);
//    forecast.insert(basetime + 2h, 2.5);
//    forecast.insert(basetime + 3h, 3.5);
//    forecast.insert(basetime + 4h, 4.5);
//
//    db->addPredictions(basetime, "model1", "Ukkel", "pm10", "dayavg", 4_d, forecast);
//
//    auto startTime = basetime + 1h;
//    auto endTime = basetime + 3h;
//    auto results = db->getPredictions(startTime, endTime, "model1", "Ukkel", "pm10", "dayavg", 4_d);
//
//    EXPECT_THAT(results.datetimes(), ContainerEq(std::vector<chrono::date_time>{
//        basetime + 1h,
//        basetime + 2h,
//        basetime + 3h
//    }));
//    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ 1.5, 2.5, 3.5 }));
//}
//
//TEST_F(SqlBufferTest, GetInvalidPrediction)
//{
//    auto now = std::chrono::system_clock::now();
//    EXPECT_THROW(db->getPrediction(now, "model1", "Ukkel", "pm10", "dayavg", 4_d), RunTimeException);
//}

}
}
