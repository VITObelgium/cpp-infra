#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "AQNetwork.h"
#include "AQNetworkProvider.h"
#include "DateTime.h"
#include "EngineMock.h"
#include "RioObsProvider.h"

#include "infra/filesystem.h"
#include "infra/xmldocument.h"
#include "tools/FileTools.h"

#include <sstream>

namespace opaq {
namespace test {

using namespace date;
using namespace inf;
using namespace chrono;
using namespace testing;
using namespace std::chrono_literals;

static const std::string s_station = "40AB01";

class AQNetworkProviderMock : public AQNetworkProvider
{
public:
    MOCK_METHOD3(configure, void(const inf::XmlNode&, const std::string&, IEngine&));
    MOCK_METHOD0(getAQNetwork, AQNetwork&());
};

class RioObsProviderTest : public Test
{
protected:
    RioObsProviderTest()
    {
        _aqNetwork.addStation(Station(s_station, "Ukkel", "123", {Pollutant(81102, "pm10", "ug/m3", "PM with diameter below 10 micron")}));

        EXPECT_CALL(_aqProviderMock, getAQNetwork()).Times(AnyNumber()).WillRepeatedly(ReturnRef(_aqNetwork));

        configure(60min);
    }

    void configure(std::chrono::minutes resolution)
    {
        auto configXml = fmt::format(
            "<?xml version=\"1.0\"?>"
            "  <config>"
            "    <resolution>{}</resolution>"
            "    <file_pattern>./%pol%_data_rio.txt</file_pattern>"
            "</config>",
            resolution.count());

        auto doc    = XmlDocument::load_from_string(configXml.c_str());
        auto config = doc.child("config");

        _obsProvider.configure(config, "obsprovider", _engineMock);
        _obsProvider.setAQNetworkProvider(_aqProviderMock);
    }

    AQNetwork _aqNetwork;
    EngineMock _engineMock;
    AQNetworkProviderMock _aqProviderMock;
    RioObsProvider _obsProvider;
};

TEST_F(RioObsProviderTest, GetTimeResolution)
{
    configure(120min);
    EXPECT_EQ(2h, _obsProvider.getTimeResolution());
}

TEST_F(RioObsProviderTest, GetValues)
{
    std::stringstream ss;
    ss << "40AB01 20090101      1     2     3     4     5     6     7     8     9    10    11    12    13    14    15    16    17    18    19    20    21    22    23    24    25    26    22\n"
       << "40AB01 20090102    129    89    80   129   101    93    87    81    77    74    72    69    71    73    70    69    69    68    66    64    73    85    85    88    94    82    80\n"
       << "40AB01 20090103     42    38    29    39    41    42    37    33    34    37    40    32    35    38    36    19 -9999 -9999 -9999    13    16    16    16    17    20    19    22\n";

    file::write_as_text("pm10_data_rio.txt", ss.str());

    auto values = _obsProvider.getValues(make_date_time(2009_y / jan / 02), make_date_time(2009_y / jan / 03), s_station, "pm10", Aggregation::Max1h);
    EXPECT_EQ(2u, values.size());
    EXPECT_THAT(values.values(), ContainerEq(std::vector<double>{129.0, 42.0}));
    EXPECT_THAT(values.datetimes(), ContainerEq(std::vector<date_time>{make_date_time(2009_y / jan / 02), make_date_time(2009_y / jan / 03)}));

    values = _obsProvider.getValues(make_date_time(2009_y / jan / 02), make_date_time(2009_y / jan / 03), s_station, "pm10", Aggregation::Max8h);
    EXPECT_EQ(2u, values.size());
    EXPECT_THAT(values.values(), ContainerEq(std::vector<double>{89.0, 38.0}));
    EXPECT_THAT(values.datetimes(), ContainerEq(std::vector<date_time>{make_date_time(2009_y / jan / 02), make_date_time(2009_y / jan / 03)}));

    values = _obsProvider.getValues(make_date_time(2009_y / jan / 02), make_date_time(2009_y / jan / 03), s_station, "pm10", Aggregation::DayAvg);
    EXPECT_EQ(2u, values.size());
    EXPECT_THAT(values.values(), ContainerEq(std::vector<double>{80.0, 29.0}));
    EXPECT_THAT(values.datetimes(), ContainerEq(std::vector<date_time>{make_date_time(2009_y / jan / 02), make_date_time(2009_y / jan / 03)}));

    values = _obsProvider.getValues(make_date_time(2009_y / jan / 02), make_date_time(2009_y / jan / 03) + 23h, s_station, "pm10", Aggregation::None);
    EXPECT_EQ(48u, values.size());
    EXPECT_THAT(values.values(), ContainerEq(std::vector<double>{129, 101, 93, 87, 81, 77, 74, 72, 69, 71, 73, 70, 69, 69, 68, 66, 64, 73, 85, 85, 88, 94, 82, 80,
                                     39, 41, 42, 37, 33, 34, 37, 40, 32, 35, 38, 36, 19, -9999, -9999, -9999, 13, 16, 16, 16, 17, 20, 19, 22}));

    auto date = make_date_time(2009_y / jan / 02);
    for (auto i = 0; i < 48; ++i) {
        EXPECT_EQ(date, values.datetime(i));
        date += 1h;
    }
}

TEST_F(RioObsProviderTest, GetValuesInvalidStation)
{
    EXPECT_TRUE(_obsProvider.getValues(make_date_time(2009_y / jan / 02), make_date_time(2009_y / jan / 03), "InvalidStation", "pm10", Aggregation::None).isEmpty());
}

TEST_F(RioObsProviderTest, NoStationData)
{
    FileTools::remove("pm10_data_rio.txt");
    EXPECT_TRUE(_obsProvider.getValues(make_date_time(2009_y / jan / 02), make_date_time(2009_y / jan / 03), s_station, "pm10", Aggregation::None).isEmpty());
}
}
}
