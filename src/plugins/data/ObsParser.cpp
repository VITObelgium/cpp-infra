#include "ObsParser.h"

#include "tools/StringTools.h"
#include "tools/AQNetworkTools.h"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <fstream>

namespace OPAQ
{

//class string_splitter
//{
//public:
//    string_splitter(boost::string_ref src, boost::string_ref sep) : _src(src), _sep(sep) {}
//    string_splitter(const std::string&& src, boost::string_ref sep) = delete;
//
//    boost::string_ref next() const
//    {
//        const auto pos = _src.substr(_pos).find_first_of(_sep);
//        const auto old_pos = _pos;
//        if (pos != -1)
//            _pos += pos + 1;
//        else
//            _finished = true;
//        return _src.substr(old_pos, pos);
//    }
//
//    boost::string_ref rest() const { return _src.substr(_pos); }
//    bool finished() const noexcept { return _finished; }
//
//    class const_iterator : public boost::iterator_facade<const_iterator, boost::string_ref, boost::single_pass_traversal_tag>
//    {
//    public:
//        explicit const_iterator(const string_splitter* sp) : _splitter(sp) {}
//
//        const_iterator& operator++()
//        {
//            if (!_splitter->finished())
//                _value = _splitter->next();
//            else
//                _splitter = nullptr;
//            return *this;
//        }
//        reference operator*() { return _value; }
//
//        bool operator !=(const const_iterator & other) const
//        {
//            return _splitter != other._splitter;
//        }
//
//    private:
//        const string_splitter* _splitter;
//        boost::string_ref _value;
//    };
//
//private:
//    boost::string_ref _src;
//    boost::string_ref _sep;
//    mutable size_t _pos = 0;
//    mutable bool _finished = false;
//};


// parse the file and read in the pollutant //
// no need to specify the aggregation time since we load the whole file in memory and build the
// map in one go...
std::map<Aggregation::Type, std::map<std::string, TimeSeries<double>>> readObservationsFile(std::istream& file,
                                                                                            const AQNetwork& aqNetwork,
                                                                                            uint32_t numberOfValues,
                                                                                            const TimeInterval& timeResolution)
{
    std::map<Aggregation::Type, std::map<std::string, TimeSeries<double>>> result;

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string line;
    while (getline(buffer, line))
    {
        // line format:
        // stationCode YYYYMMDD m1 m8 da hour0 hour1, ..., hour23

        boost::char_separator<char> sep(" ");
        boost::tokenizer<boost::char_separator<char>> tok(line, sep);

        auto iter = tok.begin();

        auto station = *iter;

        if (aqNetwork.containsStation(station))
        {
            ++iter;

            // only parse lines within the time interval of the buffer
            DateTime begin(atoi(iter->substr(0, 4).c_str()),
                           atoi(iter->substr(4, 2).c_str()),
                           atoi(iter->substr(6, 2).c_str()), 0, 0, 0);

            ++iter;

            // get the different aggregations...
            result[Aggregation::Max1h][station].insert(begin, boost::lexical_cast<float>((*iter++).c_str())); // 3rd column is daily max
            result[Aggregation::Max8h][station].insert(begin, boost::lexical_cast<float>((*iter++).c_str())); // 4th column is max 8h
            result[Aggregation::DayAvg][station].insert(begin, boost::lexical_cast<float>((*iter++).c_str())); // 5th column is daily avg

            // get the hourly values, no aggregation
            auto& ts = result[Aggregation::None][station];
            ts.reserve(numberOfValues);
            for (; iter != tok.end(); ++iter)
            {
                std::string s = iter->c_str();
                ts.insert(begin, boost::lexical_cast<float>(iter->c_str()));
                begin = begin + timeResolution;
            }
        }
        
        //if (tokens.size() != (5 + numberOfValues))
        //    throw RunTimeException("format does not match the configuration");
    }

    return result;
}

}
