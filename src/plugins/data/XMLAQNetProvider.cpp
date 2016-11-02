#include "XMLAQNetProvider.h"


#include "Station.h"
#include "Engine.h"
#include "Exceptions.h"
#include "PluginRegistration.h"
#include "tools/StringTools.h"

#include <tinyxml.h>

namespace opaq
{

XMLAQNetProvider::XMLAQNetProvider()
: _logger("OPAQ::XMLAQNetProvider")
{
}

std::string XMLAQNetProvider::name()
{
    return "xmlaqnetprovider";
}

void XMLAQNetProvider::configure(TiXmlElement* cnf, const std::string& componentName, IEngine& engine)
{
    setName(componentName);

    // Here we assume we recieve the <config> element which should define the AQNetwork...
    TiXmlElement* netEl = cnf->FirstChildElement("network");
    if (!netEl) {
        _logger->error("network element not found in configuration");
        throw BadConfigurationException("network element not found in configuration");
    }

    // loop over station elements
    long stID          = 1; // here we assign a unique ID to each station
    TiXmlElement* stEl = netEl->FirstChildElement("station");
    while (stEl)
    {
        std::string name, meteoId, desc;
        double x, y, z;

        // get station attributes
        if ((stEl->QueryStringAttribute("name", &name) != TIXML_SUCCESS) || (stEl->QueryDoubleAttribute("x", &x) != TIXML_SUCCESS) || (stEl->QueryDoubleAttribute("y", &y) != TIXML_SUCCESS))
            throw BadConfigurationException("station " + name + " should at least have name, x and y defined");

        // z is optional, default is 0.
        if (stEl->QueryDoubleAttribute("z", &z) != TIXML_SUCCESS) z = 0;

        // meteo is optional, default is ""
        if (stEl->QueryStringAttribute("meteo", &meteoId) != TIXML_SUCCESS)
            meteoId = "";

        // meteo is optional, default is ""
        if (stEl->QueryStringAttribute("desc", &desc) != TIXML_SUCCESS)
            meteoId = "";

        // create station and push back to network
        auto st = std::make_unique<Station>(name, desc, meteoId);
        st->setId(stID++); // and increment Id after assignment
        st->setX(x);
        st->setY(y);
        st->setZ(z);

        // get pollutant list from stEl->GetText(); via string tokenizer
        if (stEl->GetText()) {

            std::string str = stEl->GetText();

            auto pol_list = StringTools::tokenize(str, ",:;| \t", 6);
            for (auto& pol : pol_list)
            {
                // add to the pollutants list for this station
                st->addPollutant(engine.pollutantManager().find(pol));
            }
        }

        _net.addStation(std::move(st));

        stEl = stEl->NextSiblingElement("station");
    } /* end while loop over station elements */

    if (_net.getStations().empty())
    {
        throw BadConfigurationException("no stations defined in network");
    }
}

AQNetwork& XMLAQNetProvider::getAQNetwork()
{
    return _net;
}

OPAQ_REGISTER_STATIC_PLUGIN(XMLAQNetProvider)

}
