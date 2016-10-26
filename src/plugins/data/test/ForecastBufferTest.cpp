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
    , _noData(_buffer->getNoData())
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
    double _noData;
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
        basetime2 + fcHor
    }));
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ 1.5, 2.5 }));
}

TYPED_TEST(ForecastBufferTest, SetGetForecastSingleValue)
{
    auto basetime1 = make_date_time(2015_y / jan / 1);
    auto basetime2 = make_date_time(2015_y / jan / 2);

    auto fcHor = 1_d;

    TimeSeries<double> forecast1;
    forecast1.insert(basetime1 + fcHor, 1.5);

    this->_buffer->setCurrentModel("model1");
    this->_buffer->setValues(basetime1, forecast1, "Ukkel", "pm10", Aggregation::DayAvg);

    auto results = this->_buffer->getForecastValues(fcHor, basetime1 + fcHor, basetime1 + fcHor, "Ukkel", "pm10", Aggregation::DayAvg);

    EXPECT_THAT(results.datetimes(), ContainerEq(std::vector<chrono::date_time>{
        basetime1 + fcHor,
    }));
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ 1.5 }));
}

TYPED_TEST(ForecastBufferTest, SetGetForecastValuesCompletelyOutOfRangeAtEnd)
{
    auto basetime = make_date_time(2015_y / jan / 1);
    auto fcHor = 1_d;

    TimeSeries<double> forecast;
    forecast.insert(basetime + fcHor, 1.5);
    forecast.insert(basetime + fcHor + 24h, 2.5);

    this->_buffer->setCurrentModel("model1");
    this->_buffer->setValues(basetime, forecast, "Ukkel", "pm10", Aggregation::DayAvg);

    auto results = this->_buffer->getForecastValues(fcHor, basetime + fcHor + 48h, basetime + fcHor + 72h, "Ukkel", "pm10", Aggregation::DayAvg);

    EXPECT_THAT(results.datetimes(), ContainerEq(std::vector<chrono::date_time>{
        basetime + fcHor + 48h,
        basetime + fcHor + 72h
    }));
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ _noData, _noData }));
}

TYPED_TEST(ForecastBufferTest, SetGetForecastValuesCompletelyOutOfRangeAtFront)
{
    auto basetime = make_date_time(2015_y / jan / 1);
    auto fcHor = 1_d;

    TimeSeries<double> forecast;
    forecast.insert(basetime + fcHor, 1.5);
    forecast.insert(basetime + fcHor + 24h, 2.5);

    this->_buffer->setCurrentModel("model1");
    this->_buffer->setValues(basetime, forecast, "Ukkel", "pm10", Aggregation::DayAvg);

    auto results = this->_buffer->getForecastValues(fcHor, basetime - 24h, basetime, "Ukkel", "pm10", Aggregation::DayAvg);

    EXPECT_THAT(results.datetimes(), ContainerEq(std::vector<chrono::date_time>{
        basetime - 24h,
        basetime
    }));
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ _noData, _noData }));
}

TYPED_TEST(ForecastBufferTest, SetGetForecastValuesCompletelyPartiallyOutOfRangeAtFront)
{
    auto basetime = make_date_time(2015_y / jan / 1);
    auto fcHor = 1_d;

    TimeSeries<double> forecast;
    forecast.insert(basetime + fcHor, 1.5);
    forecast.insert(basetime + fcHor + 24h, 2.5);

    this->_buffer->setCurrentModel("model1");
    this->_buffer->setValues(basetime, forecast, "Ukkel", "pm10", Aggregation::DayAvg);

    auto results = this->_buffer->getForecastValues(fcHor, basetime, basetime + fcHor, "Ukkel", "pm10", Aggregation::DayAvg);

    EXPECT_THAT(results.datetimes(), ContainerEq(std::vector<chrono::date_time>{
        basetime,
        basetime + fcHor
    }));
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ _noData, 1.5 }));
}

TYPED_TEST(ForecastBufferTest, SetGetForecastValuesCompletelyPartiallyOutOfRangeAtEnd)
{
    auto basetime = make_date_time(2015_y / jan / 1);
    auto fcHor = 1_d;

    TimeSeries<double> forecast;
    forecast.insert(basetime + fcHor, 1.5);

    this->_buffer->setCurrentModel("model1");
    this->_buffer->setValues(basetime, forecast, "Ukkel", "pm10", Aggregation::DayAvg);

    auto results = this->_buffer->getForecastValues(fcHor, basetime + fcHor, basetime + fcHor + 24h, "Ukkel", "pm10", Aggregation::DayAvg);

    EXPECT_THAT(results.datetimes(), ContainerEq(std::vector<chrono::date_time>{
        basetime + fcHor,
        basetime + fcHor + 24h
    }));
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ 1.5, _noData }));
}

TYPED_TEST(ForecastBufferTest, OverwriteValue)
{
    auto basetime = make_date_time(2015_y / jan / 1);
    auto fcHor = 1_d;

    TimeSeries<double> forecast1, forecast2;
    forecast1.insert(basetime + fcHor, 1.5);
    forecast2.insert(basetime + fcHor, 2.5);

    this->_buffer->setCurrentModel("model1");
    this->_buffer->setValues(basetime, forecast1, "Ukkel", "pm10", Aggregation::DayAvg);
    this->_buffer->setValues(basetime, forecast2, "Ukkel", "pm10", Aggregation::DayAvg);

    auto results = this->_buffer->getForecastValues(fcHor, basetime + fcHor, basetime + fcHor, "Ukkel", "pm10", Aggregation::DayAvg);

    EXPECT_THAT(results.datetimes(), ContainerEq(std::vector<chrono::date_time>{
        basetime + fcHor
    }));
    EXPECT_THAT(results.values(), ContainerEq(std::vector<double>{ 2.5 }));
}

TYPED_TEST(ForecastBufferTest, SecondTimeBeforeFirstTime)
{
    auto basetime = make_date_time(2015_y / jan / 1);
    EXPECT_THROW(this->_buffer->getForecastValues(1_d, basetime + 24h, basetime, "Ukkel", "pm10", Aggregation::DayAvg), RunTimeException);
}

}
}
