#include "StationInfoProvider.h"
#include "PluginRegistration.h"

#include "infra/configdocument.h"
#include "infra/filesystem.h"
#include "infra/string.h"

#include <boost/lexical_cast.hpp>
#include <fstream>

namespace opaq {

using namespace infra;
using namespace std::chrono_literals;

static const char* s_pollutantPlaceholder = "%pol%";
static const char* s_gisTypePlaceholder   = "%gis%";

StationInfoProvider::StationInfoProvider()
: _logger("StationInfoProvider")
, _configured(false)
{
}

std::string StationInfoProvider::name()
{
    return "stationinfoprovider";
}

void StationInfoProvider::configure(const ConfigNode& configuration, const std::string& componentName, IEngine&)
{
    setName(componentName);

    _pattern = std::string(configuration.child("file_pattern").value());
    _stations.clear();
    _configured = true;
}

std::vector<Station> StationInfoProvider::getStations(Pollutant pollutant, const std::string& gisType)
{
    if (_stations[pollutant.getName()][gisType].empty()) {
        readFile(pollutant, gisType);
    }

    return _stations[pollutant.getName()][gisType];
}

void StationInfoProvider::readFile(Pollutant pollutant, const std::string& gisType)
{
    // create file name & open file stream
    std::string filename = _pattern;
    str::replaceInPlace(filename, s_pollutantPlaceholder, pollutant.getName());
    str::replaceInPlace(filename, s_gisTypePlaceholder, gisType);

    try {
        auto contents = file::readAsText(filename);
        str::Splitter lineSplitter(contents, "\r\n");

        auto& stations = _stations[pollutant.getName()][gisType];

        bool first = true;
        for (auto& line : lineSplitter) {
            if (first) {
                // skip header
                first = false;
                continue;
            }

            // line format: ID STATIONCODE XLAMB YLAMB ALTITUDE TYPE BETA
            str::Splitter stationSplitter(line, " \t\r\n\f");
            auto iter = stationSplitter.begin();

            auto id          = boost::lexical_cast<long>(*iter++);
            auto stationCode = boost::lexical_cast<std::string>(*iter++);
            auto x           = boost::lexical_cast<double>(*iter++);
            auto y           = boost::lexical_cast<double>(*iter++);
            auto z           = boost::lexical_cast<double>(*iter++);

            stations.push_back(Station(id, x, y, z, std::move(stationCode), "", "", {}));
        }
    } catch (const std::exception& e) {
        _logger->warn(e.what());
    }
}

OPAQ_REGISTER_STATIC_PLUGIN(StationInfoProvider)
}
