#include "StationInfoProvider.h"
#include "PluginRegistration.h"

#include "tools/StringTools.h"
#include "tools/XmlTools.h"

#include <fstream>
#include <tinyxml.h>
#include <boost/lexical_cast.hpp>

namespace opaq
{

using namespace std::chrono_literals;

static std::string s_pollutantPlaceholder = "%pol%";
static std::string s_gisTypePlaceholder = "%gis%";

StationInfoProvider::StationInfoProvider()
: _logger("StationInfoProvider")
, _configured(false)
, _gisType("clc06d")
{
}

std::string StationInfoProvider::name()
{
    return "stationinfoprovider";
}

void StationInfoProvider::configure(TiXmlElement* cnf, const std::string& componentName, IEngine&)
{
    setName(componentName);

    _pattern = XmlTools::getChildValue<std::string>(cnf, "file_pattern");
    _stations.clear();
    _configured = true;
}

std::vector<Station> StationInfoProvider::getStations(Pollutant pollutant)
{
    if (_stations[pollutant.getName()].empty())
    {
        readFile(pollutant);
    }

    return _stations[pollutant.getName()];
}

void StationInfoProvider::readFile(Pollutant pollutant)
{
    // create file name & open file stream
    std::string filename = _pattern;
    StringTools::replaceAll(filename, s_pollutantPlaceholder, pollutant.getName());
    StringTools::replaceAll(filename, s_gisTypePlaceholder, _gisType);

    try
    {
        auto contents = FileTools::readFileContents(filename);
        StringTools::StringSplitter lineSplitter(contents, "\r\n");

        auto& stations = _stations[pollutant.getName()];

        bool first = true;
        for (auto& line : lineSplitter)
        {
            if (first)
            {
                // skipt header
                first = false;
                continue;
            }

            // line format: ID STATIONCODE XLAMB YLAMB ALTITUDE TYPE BETA
            StringTools::StringSplitter stationSplitter(line, " \t\r\n\f");
            auto iter = stationSplitter.begin();

            auto id = boost::lexical_cast<long>(*iter++);
            auto stationCode = boost::lexical_cast<std::string>(*iter++);
            auto x = boost::lexical_cast<double>(*iter++);
            auto y = boost::lexical_cast<double>(*iter++);
            auto z = boost::lexical_cast<double>(*iter++);

            ++iter; // skip type

            auto beta = boost::lexical_cast<double>(*iter);

            Station station(std::move(stationCode), "", "", beta);
            station.setX(x);
            station.setY(y);
            station.setZ(z);
            station.setId(id);

            stations.push_back(std::move(station));
        }
    }
    catch (const RunTimeException& e)
    {
        _logger->warn(e.what());
    }
}

OPAQ_REGISTER_STATIC_PLUGIN(StationInfoProvider)

}
