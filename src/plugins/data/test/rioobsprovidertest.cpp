#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <sstream>

#include "TimeInterval.h"
#include "ObsParser.h"
#include "AQNetwork.h"

#include <boost/utility/string_ref.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace OPAQ
{
namespace Test
{

using namespace testing;

static const std::string s_station = "40AB01";

class ObserverVationParser : public testing::Test
{
protected:
    ObserverVationParser()
    {
        auto station = std::make_unique<Station>();
        station->setName(s_station);
        network.addStation(std::move(station));
    }

    AQNetwork network;
};

//TEST_F(ObserverVationParser, StringSplitter)
//{
//    std::string test = "1 2 ";
//    string_splitter s(test, " ");
//    std::vector<std::string> result(s.begin(), s.end());
//
//    //EXPECT_THAT(result, ContainerEq(std::vector<std::string>{"1", "2", "3", "4"}));
//    EXPECT_THAT(result, ContainerEq(std::vector<std::string>{"1", "2"}));
//}

TEST_F(ObserverVationParser, ParseFile)
{
    std::stringstream ss;
    ss << "40AB01 20090101    129    89    80   129   101    93    87    81    77    74    72    69    71    73    70    69    69    68    66    64    73    85    85    88    94    82    80\n"
       << "40AB01 20090102     42    38    29    39    41    42    37    33    34    37    40    32    35    38    36    19 -9999 -9999 -9999    13    16    16    16    17    20    19    22\n";

    auto result = readObservationsFile(ss, network, 24, TimeInterval(60, TimeInterval::Minutes));
    EXPECT_EQ(4u, result.size()); // one result for each aggregation
    EXPECT_EQ(2u, result[Aggregation::Max1h][s_station].size()); // one value for each basetime
    EXPECT_EQ(2u, result[Aggregation::Max8h][s_station].size()); // one value for each basetime
    EXPECT_EQ(2u, result[Aggregation::DayAvg][s_station].size()); // one value for each basetime
    EXPECT_EQ(48u, result[Aggregation::None][s_station].size()); // 24 values for each basetime

    EXPECT_THAT(result[Aggregation::Max1h][s_station].valueAt(DateTime(2009, 01, 01)), DoubleEq(129));
    EXPECT_THAT(result[Aggregation::Max1h][s_station].valueAt(DateTime(2009, 01, 02)), DoubleEq(42));

    EXPECT_THAT(result[Aggregation::Max8h][s_station].valueAt(DateTime(2009, 01, 01)), DoubleEq(89));
    EXPECT_THAT(result[Aggregation::Max8h][s_station].valueAt(DateTime(2009, 01, 02)), DoubleEq(38));

    EXPECT_THAT(result[Aggregation::DayAvg][s_station].valueAt(DateTime(2009, 01, 01)), DoubleEq(80));
    EXPECT_THAT(result[Aggregation::DayAvg][s_station].valueAt(DateTime(2009, 01, 02)), DoubleEq(29));

    std::vector<double> values = { 129, 101, 93, 87, 81, 77, 74, 72, 69, 71, 73, 70, 69, 69, 68, 66, 64, 73, 85, 85, 88, 94, 82, 80, 
                                   39, 41, 42, 37, 33, 34, 37, 40, 32, 35, 38, 36, 19, -9999, -9999, -9999, 13, 16, 16, 16, 17, 20, 19, 22 };
    
    EXPECT_THAT(result[Aggregation::None][s_station].values(), ContainerEq(values));

    DateTime date(2009, 01, 01);
    for (int i = 0; i < 48; ++i)
    {
        EXPECT_EQ(date, result[Aggregation::None][s_station].datetime(i));
        date.addHours(1);
    }
}

TEST_F(ObserverVationParser, ParseInvalidFile)
{
    std::stringstream ss;
    ss << "40AB01 20090101    129    89    80   129   101    93    87    81    77    74    72    69    71    73    70    69    69    68    66    64    73    85    85    88    94    82    80\n"
       << "40AB01 20090102     42    38    29    39    41    42    37    33    34    37    40    32    35    38    36    19 -9999 -9999 -9999    13    16    16    16    17    20    19    \n"; // one value too little

    EXPECT_THROW(readObservationsFile(ss, network, 24, TimeInterval(60, TimeInterval::Minutes)), RunTimeException);
}

// Used to check performance
TEST_F(ObserverVationParser, ParseFile1)
{
    std::ifstream fs("C:\\Work\\opaq-validation\\data\\obs\\pm10_data_rio.txt");

    AQNetwork network;
    auto station = std::make_unique<Station>();
    station->setName(s_station);
    network.addStation(std::move(station));

    auto result = readObservationsFile(fs, network, 24, TimeInterval(60, TimeInterval::Minutes));
    EXPECT_EQ(4u, result.size()); // one result for each aggregation
}

}
}