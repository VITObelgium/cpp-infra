#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "DateTime.h"
#include "EngineMock.h"
#include "StationInfoProvider.h"

#include "infra/filesystem.h"
#include "infra/xmldocument.h"
#include "tools/FileTools.h"

#include <sstream>

namespace opaq {
namespace test {

using namespace inf;
using namespace date;
using namespace chrono;
using namespace testing;
using namespace std::chrono_literals;
using namespace std::string_literals;

static const std::string s_station = "40AB01";

class StationInfoProviderTest : public Test
{
protected:
    StationInfoProviderTest()
    {
        FileTools::remove("pm10_stations_info_GIS_clc06d.txt");

        configure();
    }

    void configure()
    {
        auto configXml =
            "<?xml version=\"1.0\"?>"
            "  <config>"
            "    <file_pattern>./%pol%_stations_info_GIS_%gis%.txt</file_pattern>"
            "</config>";

        auto doc    = XmlDocument::load_from_string(configXml);
        auto config = doc.child("config");

        _stationsProvider.configure(config, "stationinfoprovider", _engineMock);
    }

    EngineMock _engineMock;
    StationInfoProvider _stationsProvider;
};

TEST_F(StationInfoProviderTest, GetStations)
{
    std::stringstream ss;
    ss << "ID  STATCODE    XLAMB   YLAMB   ALT TYPE    BETA\r\n"
       << "1	41B004	148580	171157	25.4	3	0.824880\r\n"
       << "2	41B006	150397	169802	73.0	3	0.702315\r\n"
       << "3	41B011	144338	171963	62.1	3	0.738767\r\n"
       << "4	41N043	151000	174800	14.6	5	1.035592\r\n"
       << "5	41R001	147540	171030	23.2	3	0.754189\r\n";

    file::write_as_text("pm10_stations_info_GIS_clc06d.txt", ss.str());

    auto stations = _stationsProvider.getStations(Pollutant(1, "pm10", "unit", "desc"), "clc06d");
    EXPECT_THAT(stations, ContainerEq(std::vector<Station>{
                              Station(1, 148580.0, 171157.0, 25.4, "41B004", "", "", {}),
                              Station(2, 150397.0, 169802.0, 73.0, "41B006", "", "", {}),
                              Station(3, 144338.0, 171963.0, 62.1, "41B011", "", "", {}),
                              Station(4, 151000.0, 174800.0, 14.6, "41N043", "", "", {}),
                              Station(5, 147540.0, 171030.0, 23.2, "41R001", "", "", {})}));
}

TEST_F(StationInfoProviderTest, GetStationsInvalidPollutant)
{
    EXPECT_TRUE(_stationsProvider.getStations(Pollutant(1, "o3", "unit", "desc"), "clc06d").empty());
}
}
}
