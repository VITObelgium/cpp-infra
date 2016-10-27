#include "ObsParser.h"

#include "AQNetwork.h"
#include "tools/StringTools.h"
#include "tools/FileTools.h"

#include <boost/lexical_cast.hpp>

#include <fstream>

namespace OPAQ
{

// parse the file and read in the pollutant //
// no need to specify the aggregation time since we load the whole file in memory and build the
// map in one go...
std::map<Aggregation::Type, std::map<std::string, TimeSeries<double>>> readObservationsFile(std::istream& file,
                                                                                            const AQNetwork& aqNetwork,
                                                                                            uint32_t numberOfValues,
                                                                                            std::chrono::hours timeResolution)
{
    std::map<Aggregation::Type, std::map<std::string, TimeSeries<double>>> result;

    auto contents = FileTools::readStreamContents(file);
    StringTools::StringSplitter lineSplitter(contents, "\r\n");

    for (auto& line : lineSplitter)
    {
        // line format: stationCode YYYYMMDD m1 m8 da hour0 hour1, ..., hour23

        StringTools::StringSplitter observationSplitter(line, " \t\r\n\f");

        auto iter = observationSplitter.begin();
        const auto station = std::string(iter->begin(), iter->end());

        if (aqNetwork.containsStation(station))
        {
            ++iter;

            // only parse lines within the time interval of the buffer
            auto begin = chrono::make_date_time(boost::lexical_cast<int>(iter->substr(0, 4)),
                                                boost::lexical_cast<int>(iter->substr(4, 2)),
                                                boost::lexical_cast<int>(iter->substr(6, 2)));

            ++iter;

            // get the different aggregations...
            result[Aggregation::Max1h][station].insert(begin, boost::lexical_cast<double>(*iter++)); // 3rd column is daily max
            result[Aggregation::Max8h][station].insert(begin, boost::lexical_cast<double>(*iter++)); // 4th column is max 8h
            result[Aggregation::DayAvg][station].insert(begin, boost::lexical_cast<double>(*iter++)); // 5th column is daily avg

            uint32_t parsedValues = 0;

            // get the hourly values, no aggregation
            auto& ts = result[Aggregation::None][station];
            for (; iter != observationSplitter.end(); ++iter)
            {
                ts.insert(begin, boost::lexical_cast<double>(*iter));
                begin += timeResolution;
                ++parsedValues;
            }

            if (parsedValues != numberOfValues)
            {
                throw RunTimeException("format does not match the configuration");
            }
        }
    }

    return result;
}

}
