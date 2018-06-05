#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sstream>

#include "AQNetwork.h"
#include "ObsParser.h"

namespace opaq {
namespace test {

using namespace testing;
using namespace std::chrono_literals;

static const std::string s_station = "40AB01";

class RioObsProviderParserTest : public Test
{
protected:
    RioObsProviderParserTest()
    {
        network.addStation(Station(s_station, "Ukkel", "123", {}));
    }

    AQNetwork network;
};

TEST_F(RioObsProviderParserTest, ParseFile)
{
    using namespace date;
    using namespace chrono;

    std::stringstream ss;
    ss << "40AB01 20090101    129    89    80   129   101    93    87    81    77    74    72    69    71    73    70    69    69    68    66    64    73    85    85    88    94    82    80\n"
       << "40AB01 20090102     42    38    29    39    41    42    37    33    34    37    40    32    35    38    36    19 -9999 -9999 -9999    13    16    16    16    17    20    19    22\n";

    auto result = readObservationsFile(ss, network, 24, 1h);
    EXPECT_EQ(4u, result.size());                                 // one result for each aggregation
    EXPECT_EQ(2u, result[Aggregation::Max1h][s_station].size());  // one value for each basetime
    EXPECT_EQ(2u, result[Aggregation::Max8h][s_station].size());  // one value for each basetime
    EXPECT_EQ(2u, result[Aggregation::DayAvg][s_station].size()); // one value for each basetime
    EXPECT_EQ(48u, result[Aggregation::None][s_station].size());  // 24 values for each basetime

    EXPECT_THAT(result[Aggregation::Max1h][s_station].valueAt(make_date_time(2009_y / jan / 01)), DoubleEq(129));
    EXPECT_THAT(result[Aggregation::Max1h][s_station].valueAt(make_date_time(2009_y / jan / 02)), DoubleEq(42));

    EXPECT_THAT(result[Aggregation::Max8h][s_station].valueAt(make_date_time(2009_y / jan / 01)), DoubleEq(89));
    EXPECT_THAT(result[Aggregation::Max8h][s_station].valueAt(make_date_time(2009_y / jan / 02)), DoubleEq(38));

    EXPECT_THAT(result[Aggregation::DayAvg][s_station].valueAt(make_date_time(2009_y / jan / 01)), DoubleEq(80));
    EXPECT_THAT(result[Aggregation::DayAvg][s_station].valueAt(make_date_time(2009_y / jan / 02)), DoubleEq(29));

    std::vector<double> values = {129, 101, 93, 87, 81, 77, 74, 72, 69, 71, 73, 70, 69, 69, 68, 66, 64, 73, 85, 85, 88, 94, 82, 80,
        39, 41, 42, 37, 33, 34, 37, 40, 32, 35, 38, 36, 19, -9999, -9999, -9999, 13, 16, 16, 16, 17, 20, 19, 22};

    EXPECT_THAT(result[Aggregation::None][s_station].values(), ContainerEq(values));

    auto date = make_date_time(2009_y / jan / 01);
    for (auto i = 0; i < 48; ++i) {
        EXPECT_EQ(date, result[Aggregation::None][s_station].datetime(i));
        date += 1h;
    }
}

TEST_F(RioObsProviderParserTest, ParseFileCarriageReturnLineFeed)
{
    std::stringstream ss;
    ss << "40AB01 20090101    129    89    80   129   101    93    87    81    77    74    72    69    71    73    70    69    69    68    66    64    73    85    85    88    94    82    80\r\n"
       << "40AB01 20090102     42    38    29    39    41    42    37    33    34    37    40    32    35    38    36    19 -9999 -9999 -9999    13    16    16    16    17    20    19    22\r\n";

    auto result = readObservationsFile(ss, network, 24, 1h);
    EXPECT_EQ(4u, result.size());                                 // one result for each aggregation
    EXPECT_EQ(2u, result[Aggregation::Max1h][s_station].size());  // one value for each basetime
    EXPECT_EQ(2u, result[Aggregation::Max8h][s_station].size());  // one value for each basetime
    EXPECT_EQ(2u, result[Aggregation::DayAvg][s_station].size()); // one value for each basetime
    EXPECT_EQ(48u, result[Aggregation::None][s_station].size());  // 24 values for each basetime
}

TEST_F(RioObsProviderParserTest, ParseInvalidFile)
{
    std::stringstream ss;
    ss << "40AB01 20090101    129    89    80   129   101    93    87    81    77    74    72    69    71    73    70    69    69    68    66    64    73    85    85    88    94    82    80\n"
       << "40AB01 20090102     42    38    29    39    41    42    37    33    34    37    40    32    35    38    36    19 -9999 -9999 -9999    13    16    16    16    17    20    19    \n"; // one value too little

    EXPECT_THROW(readObservationsFile(ss, network, 24, 1h), RunTimeException);
}

// Used to check performance
TEST_F(RioObsProviderParserTest, DISABLED_ParseFile1)
{
    std::ifstream fs("C:\\Work\\opaq-validation\\data\\obs\\pm10_data_rio.txt");
    ASSERT_TRUE(fs.is_open());

    AQNetwork network;
    network.addStation(Station(s_station, "Ukkel", "123", {}));

    auto result = readObservationsFile(fs, network, 24, 1h);
    EXPECT_EQ(4u, result.size()); // one result for each aggregation
}
}
}
