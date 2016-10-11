#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <sstream>

#include "TimeInterval.h"
#include "ObsParser.h"
#include "AQNetwork.h"

namespace OPAQ
{
namespace Test
{

  using namespace testing;

class ObserverVationParser : public testing::Test
{
protected:
};

TEST_F(ObserverVationParser, ParseFile)
{
    std::stringstream ss;
    ss << "40AB01 20090101    129    89    80   129   101    93    87    81    77    74    72    69    71    73    70    69    69    68    66    64    73    85    85    88    94    82    80\n"
       << "40AB01 20090102     42    38    29    39    41    42    37    33    34    37    40    32    35    38    36    19 -9999 -9999 -9999    13    16    16    16    17    20    19    22\n";

    AQNetwork network;
    auto station = std::make_unique<Station>();
    station->setName("40AB01");
    network.addStation(std::move(station));

    auto result = readObservationsFile(ss, network, 24, TimeInterval(60, TimeInterval::Minutes));
    EXPECT_EQ(4u, result.size()); // one result for each aggregation
    EXPECT_EQ(2u, result[Aggregation::Max1h]["40AB01"].size()); // one value for each basetime
    EXPECT_EQ(2u, result[Aggregation::Max8h]["40AB01"].size()); // one value for each basetime
    EXPECT_EQ(2u, result[Aggregation::DayAvg]["40AB01"].size()); // one value for each basetime
    EXPECT_EQ(48u, result[Aggregation::None]["40AB01"].size()); // 24 values for each basetime

    EXPECT_THAT(result[Aggregation::Max1h]["40AB01"].valueAt(DateTime(2009, 01, 01)), DoubleEq(129));
    EXPECT_THAT(result[Aggregation::Max1h]["40AB01"].valueAt(DateTime(2009, 01, 02)), DoubleEq(42));

    EXPECT_THAT(result[Aggregation::Max8h]["40AB01"].valueAt(DateTime(2009, 01, 01)), DoubleEq(89));
    EXPECT_THAT(result[Aggregation::Max8h]["40AB01"].valueAt(DateTime(2009, 01, 02)), DoubleEq(38));

    EXPECT_THAT(result[Aggregation::DayAvg]["40AB01"].valueAt(DateTime(2009, 01, 01)), DoubleEq(80));
    EXPECT_THAT(result[Aggregation::DayAvg]["40AB01"].valueAt(DateTime(2009, 01, 02)), DoubleEq(29));

    std::vector<double> values = { 129, 101, 93, 87, 81, 77, 74, 72, 69, 71, 73, 70, 69, 69, 68, 66, 64, 73, 85, 85, 88, 94, 82, 80, 
                                   39, 41, 42, 37, 33, 34, 37, 40, 32, 35, 38, 36, 19, -9999, -9999, -9999, 13, 16, 16, 16, 17, 20, 19, 22 };
    
    EXPECT_THAT(result[Aggregation::None]["40AB01"].values(), ContainerEq(values));

    DateTime date(2009, 01, 01);
    for (int i = 0; i < 48; ++i)
    {
        EXPECT_EQ(date, result[Aggregation::None]["40AB01"].datetime(i));
        date.addHours(1);
    }
}

TEST_F(ObserverVationParser, ParseFile1)
{
    std::ifstream fs("C:\\Work\\opaq-validation\\data\\obs\\pm10_data_rio.txt");

    AQNetwork network;
    auto station = std::make_unique<Station>();
    station->setName("40AB01");
    network.addStation(std::move(station));

    auto result = readObservationsFile(fs, network, 24, TimeInterval(60, TimeInterval::Minutes));
    EXPECT_EQ(4u, result.size()); // one result for each aggregation
}

}
}