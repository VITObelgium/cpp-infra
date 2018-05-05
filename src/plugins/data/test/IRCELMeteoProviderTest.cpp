#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "AQNetwork.h"
#include "AQNetworkProvider.h"
#include "DateTime.h"
#include "EngineMock.h"
#include "IRCELMeteoProvider.h"

#include "infra/configdocument.h"
#include "tools/FileTools.h"

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <sstream>

namespace opaq {
namespace test {

using namespace date;
using namespace infra;
using namespace chrono;
using namespace testing;
using namespace std::chrono_literals;

static const std::string s_station = "40AB01";

class IRCELMeteoProviderTest : public Test
{
protected:
    IRCELMeteoProviderTest()
    {
        FileTools::del("BEL_096_4800_P07.txt");
        FileTools::del("BEL_096_4800_P08.txt");
        FileTools::del("BEL_096_4800_P07.txt.gz");

        configure(3h, "./BEL_%meteo%_%param%.txt");
    }

    static const char* referenceData()
    {
        return "20150101        10.572        17.973        65.667        48.384       280.337       278.747       235.626       291.525\n"
               "20150102       393.592       566.608       595.988       582.382       403.172       498.695        38.524        15.083\n"
               "20150103        64.291       262.974       367.226      1158.536      1410.489      1273.104      1470.478      1202.126\n"
               "20150104       803.952       498.422       241.122       189.763       244.497       337.089       312.469       361.332\n";
    }

    void configure(std::chrono::hours resolution, const std::string& filePattern)
    {
        auto configXml = fmt::format(
            "<?xml version=\"1.0\"?>"
            "  <config>"
            "    <resolution>{}</resolution>"
            "    <file_pattern>{}</file_pattern>"
            "    <parameters>"
            "      <parameter id = \"P07\" alias=\"BLH\" nodata=\"-999\"/>"
            "    </parameters>"
            "</config>",
            resolution.count(), filePattern);

        auto doc    = ConfigDocument::loadFromString(configXml);
        auto config = doc.child("config");

        _meteoProvider.configure(config, "meteoprovider", _engineMock);
        _meteoProvider.setBaseTime(make_date_time(2015_y / jan / 01));
    }

    EngineMock _engineMock;
    IRCELMeteoProvider _meteoProvider;
};

TEST_F(IRCELMeteoProviderTest, GetTimeResolution)
{
    configure(4h, "./BEL_%meteo%_%param%.txt");
    EXPECT_EQ(4h, _meteoProvider.getTimeResolution());
}

TEST_F(IRCELMeteoProviderTest, GetValues)
{
    FileTools::writeTextFile("BEL_096_4800_P07.txt", referenceData());

    auto values = _meteoProvider.getValues(make_date_time(2015_y / jan / 02), make_date_time(2015_y / jan / 03) + 23h, "096_4800", "P07");
    EXPECT_EQ(16u, values.size());
    EXPECT_THAT(values.values(), ContainerEq(std::vector<double>{393.592, 566.608, 595.988, 582.382, 403.172, 498.695, 38.524, 15.083,
                                     64.291, 262.974, 367.226, 1158.536, 1410.489, 1273.104, 1470.478, 1202.126}));

    auto date = make_date_time(2015_y / jan / 02);
    for (auto i = 0; i < 16; ++i) {
        EXPECT_EQ(date, values.datetime(i));
        date += 3h;
    }
}

TEST_F(IRCELMeteoProviderTest, GetValuesMultiplePollutants)
{
    FileTools::writeTextFile("BEL_096_4800_P07.txt", referenceData());
    FileTools::writeTextFile("BEL_096_4800_P08.txt", referenceData());

    auto values = _meteoProvider.getValues(make_date_time(2015_y / jan / 02), make_date_time(2015_y / jan / 02) + 23h, "096_4800", "P07");
    EXPECT_EQ(8u, values.size());
    EXPECT_THAT(values.values(), ContainerEq(std::vector<double>{393.592, 566.608, 595.988, 582.382, 403.172, 498.695, 38.524, 15.083}));

    auto date = make_date_time(2015_y / jan / 02);
    for (auto i = 0; i < 8; ++i) {
        EXPECT_EQ(date, values.datetime(i));
        date += 3h;
    }

    values = _meteoProvider.getValues(make_date_time(2015_y / jan / 02), make_date_time(2015_y / jan / 02) + 23h, "096_4800", "P08");
    EXPECT_EQ(8u, values.size());
    EXPECT_THAT(values.values(), ContainerEq(std::vector<double>{393.592, 566.608, 595.988, 582.382, 403.172, 498.695, 38.524, 15.083}));
}

TEST_F(IRCELMeteoProviderTest, GetValuesCompressed)
{
    configure(3h, "./BEL_%meteo%_%param%.txt.gz");

    std::stringstream ss;
    ss << referenceData();

    {
        std::ofstream outStream("BEL_096_4800_P07.txt.gz", std::ios::trunc);
        boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
        in.push(boost::iostreams::gzip_compressor());
        in.push(ss);
        boost::iostreams::copy(in, outStream);
    }

    auto values = _meteoProvider.getValues(make_date_time(2015_y / jan / 02), make_date_time(2015_y / jan / 03) + 23h, "096_4800", "P07");
    EXPECT_EQ(16u, values.size());
    EXPECT_THAT(values.values(), ContainerEq(std::vector<double>{393.592, 566.608, 595.988, 582.382, 403.172, 498.695, 38.524, 15.083,
                                     64.291, 262.974, 367.226, 1158.536, 1410.489, 1273.104, 1470.478, 1202.126}));

    auto date = make_date_time(2015_y / jan / 02);
    for (auto i = 0; i < 16; ++i) {
        EXPECT_EQ(date, values.datetime(i));
        date += 3h;
    }
}

// Check with Bino, this seems to be incorrectly implemented
TEST_F(IRCELMeteoProviderTest, DISABLED_GetValuesOutOfRangeUsePreviousDay)
{
    FileTools::writeTextFile("BEL_096_4800_P07.txt", referenceData());

    auto values = _meteoProvider.getValues(make_date_time(2015_y / jan / 05), make_date_time(2015_y / jan / 05) + 23h, "096_4800", "P07");
    EXPECT_EQ(8u, values.size());
    EXPECT_THAT(values.values(), ContainerEq(std::vector<double>{803.952, 498.422, 241.122, 189.763, 244.497, 337.089, 312.469, 361.332}));

    auto date = make_date_time(2015_y / jan / 04);
    for (auto i = 0; i < 8; ++i) {
        EXPECT_EQ(date, values.datetime(i));
        date += 3h;
    }
}

TEST_F(IRCELMeteoProviderTest, GetValuesInvalidParameter)
{
    EXPECT_TRUE(_meteoProvider.getValues(make_date_time(2015_y / jan / 02), make_date_time(2015_y / jan / 03) + 23h, "096_4800", "P08").isEmpty());
}

TEST_F(IRCELMeteoProviderTest, GetValuesInvalidMeteoId)
{
    EXPECT_TRUE(_meteoProvider.getValues(make_date_time(2015_y / jan / 02), make_date_time(2015_y / jan / 03) + 23h, "InvalidId", "P07").isEmpty());
}
}
}
