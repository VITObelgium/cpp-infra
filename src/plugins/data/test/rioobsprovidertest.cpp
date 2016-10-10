#include <gtest/gtest.h>

#include <sstream>

#include "TimeInterval.h"
#include "ObsParser.h"
#include "AQNetwork.h"

namespace OPAQ
{
namespace Test
{

using namespace ::testing;

class ObserverVationParser : public Test
{
protected:
};

TEST_F(ObserverVationParser, ParseFile)
{
    std::stringstream ss;
    ss << "40AB01 20090101    129    89    80   129   101    93    87    81    77    74    72    69    71    73    70    69    69    68    66    64    73    85    85    88    94    82    80\n"
       << "40AB01 20090102     93    81    58    81    88    93    87    82    79    71    65    59    53    52    58    56    61    66    65    51    32    32    24    29    35    35    34\n"
       << "40AB01 20090103     99    88    66    34    37    38    39    40    45    48    49    53    89    68    67    75    76    69    61    85    95    99    94    93    93    82    63\n"
       << "40AB01 20090104     48    43    41    44    44    48    46    42    40    41    39    36    37    40    41    45    46    44    44    43    39    41    37    34    36    36    39\n"
       << "40AB01 20090105     42    38    29    39    41    42    37    33    34    37    40    32    35    38    36    19 -9999 -9999 -9999    13    16    16    16    17    20    19    22\n"
       << "40AB01 20090106     76    50    33    22    21    22    23    23    22    22    24    24    27    24    26    26    30    26    23    31    33    34    44    51    64    76    66\n"
       << "40AB01 20090107    123   102    74    84    87    66    67    55    44    40    42    44    45    40    45    69    80    77    81    87    91    86    98    99   107   121   123\n"
       << "40AB01 20090108    122    87    65   122   113   108   106    86    51    48    63    64    41    51    61    60    56    46    52    61    60    57    54    50    47    50    45\n"
       << "40AB01 20090109    156   128    93    46    50    53    51    53    52    54    64    62    65   122 -9999 -9999   130    97   119   122   103   115   119   125   141   142   156\n"
       << "40AB01 20090110    164   132   109   164   150   144   132   129   129   111   100   109    89   106   127   122   116   120    96    77    84    98    96    80    85    78    78";

    AQNetwork network;
    auto station = std::make_unique<Station>();
    station->setName("40AB01");
    network.addStation(std::move(station));

    auto result = readObservationsFile(ss, network, 24, TimeInterval(60, TimeInterval::Minutes));
    EXPECT_EQ(4u, result.size()); // one result for each aggregation
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