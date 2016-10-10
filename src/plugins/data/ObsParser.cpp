#include "ObsParser.h"

#include "tools/StringTools.h"
#include "tools/AQNetworkTools.h"

#include <boost/tokenizer.hpp>

#include <fstream>

namespace OPAQ
{

// parse the file and read in the pollutant //
// no need to specify the aggregation time since we load the whole file in memory and build the
// map in one go...
std::map<Aggregation::Type, std::map<std::string, TimeSeries<double>>> readObservationsFile(std::istream& file,
                                                                                            const AQNetwork& aqNetwork,
                                                                                            uint32_t numberOfValues,
                                                                                            const TimeInterval& timeResolution)
{
    std::map<Aggregation::Type, std::map<std::string, TimeSeries<double>>> result;

    std::string line;
    while (getline(file, line))
    {
        /*
            * line format:
            * stationCode YYYYMMDD m1 m8 da hour0 hour1, ..., hour23
            */

        boost::tokenizer<> tok(line);
        auto iter = tok.begin();

        auto station = *iter;

        if (aqNetwork.containsStation(station))
        {
            ++iter;

            // only parse lines within the time interval of the buffer
            DateTime begin(atoi(iter->substr(0, 4).c_str()),
                           atoi(iter->substr(4, 2).c_str()),
                           atoi(iter->substr(6, 2).c_str()), 0, 0, 0);

            // get the different aggregations...
            result[Aggregation::Max1h][station].insert(begin, atof((*iter++).c_str())); // 3rd column is daily max
            result[Aggregation::Max8h][station].insert(begin, atof((*iter++).c_str())); // 4th column is max 8h
            result[Aggregation::DayAvg][station].insert(begin, atof((*iter++).c_str())); // 5th column is daily avg

            // get the hourly values, no aggregation
            auto& ts = result[Aggregation::None][station];
            ts.reserve(numberOfValues);
            for (; iter != tok.end(); ++iter)
            {
                ts.insert(begin, atof(iter->c_str()));
                begin = begin + timeResolution;
            }
        }
        
        //if (tokens.size() != (5 + numberOfValues))
        //    throw RunTimeException("format does not match the configuration");
    }

    return result;
}

}
