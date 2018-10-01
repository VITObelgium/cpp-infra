#include "XMLAQNetProvider.h"

#include "Engine.h"
#include "Exceptions.h"
#include "PollutantManager.h"
#include "Station.h"
#include "infra/configdocument.h"
#include "infra/log.h"
#include "infra/string.h"

namespace opaq {

using namespace inf;

static const LogSource s_logSrc("XMLAQNetProvider");

std::string XMLAQNetProvider::name()
{
    return "xmlaqnetprovider";
}

void XMLAQNetProvider::configure(const ConfigNode& configuration, const std::string& componentName, IEngine& engine)
{
    setName(componentName);

    // Here we assume we recieve the <config> element which should define the AQNetwork...
    auto netEl = configuration.child("network");
    if (!netEl) {
        Log::error(s_logSrc, "network element not found in configuration");
        throw BadConfigurationException("network element not found in configuration");
    }

    // loop over station elements
    long stID = 1; // here we assign a unique ID to each station
    for (auto& stEl : netEl.children("station")) {
        auto name    = std::string(stEl.attribute("name"));
        auto meteoId = std::string(stEl.attribute("meteo"));
        auto desc    = std::string(stEl.attribute("desc"));

        auto x = stEl.attribute<double>("x");
        auto y = stEl.attribute<double>("y");
        auto z = stEl.attribute<double>("z");

        // create station and push back to network
        std::vector<Pollutant> pollutants;

        // get pollutant list from stEl->GetText(); via string tokenizer
        auto val      = stEl.value();
        auto pol_list = str::split(val, ",:;| \t", str::SplitOpt::DelimiterIsCharacterArray);
        for (auto& pol : pol_list) {
            // add to the pollutants list for this station
            pollutants.push_back(engine.pollutantManager().find(pol));
        }

        _net.addStation(Station(stID++, *x, *y, z.value_or(0), name, desc, meteoId, std::move(pollutants)));
    } /* end while loop over station elements */

    if (_net.getStations().empty()) {
        throw BadConfigurationException("no stations defined in network");
    }
}

AQNetwork& XMLAQNetProvider::getAQNetwork()
{
    return _net;
}
}
