#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>

#include "XMLAQNetProvider.h"

namespace OPAQ {

XMLAQNetProvider::XMLAQNetProvider() {
}

XMLAQNetProvider::~XMLAQNetProvider() {
  // clean up
  std::vector<Station*> * stations = &(_net.getStations());
  std::vector<Station*>::iterator it = stations->begin();
  while (it != stations->end()) {
    Station * toErase = *it++;
    delete toErase;
  }
}
  
void XMLAQNetProvider::configure(TiXmlElement *cnf)
		throw (OPAQ::BadConfigurationException) {

  // Here we assume we recieve the <config> element which should define the AQNetwork...
  TiXmlElement *netEl = cnf->FirstChildElement("network");
  if (!netEl) {
    logger->error("network element not found in configuration");
    throw OPAQ::BadConfigurationException("network element not found in configuration");
  }

  // loop over station elements
  long stID = 1; // here we assign a unique ID to each station
  TiXmlElement *stEl = netEl->FirstChildElement("station");
  while (stEl) {
    std::string name, meteoId;
    double x, y, z;

    // get station attributes
    if ((stEl->QueryStringAttribute("name", &name) != TIXML_SUCCESS)
	|| (stEl->QueryDoubleAttribute("x", &x) != TIXML_SUCCESS)
	|| (stEl->QueryDoubleAttribute("y", &y) != TIXML_SUCCESS))
      throw OPAQ::BadConfigurationException("station " + name + " should at least have name, x and y defined");
    
    // z is optional, default is 0.
    if (stEl->QueryDoubleAttribute("z", &z) != TIXML_SUCCESS) z = 0;

    // meteo is optional, default is ""
    if (stEl->QueryStringAttribute("meteo", &meteoId) != TIXML_SUCCESS)
      meteoId = "";

    // create station and push back to network
    OPAQ::Station *st = new OPAQ::Station();
    st->setName(name);
    st->setId(stID++); // and increment Id after assignment
    st->setX(x);
    st->setY(y);
    st->setZ(z);
    st->setMeteoId(meteoId);

    // get pollutant list from stEl->GetText(); via string tokenizer
    if ( stEl->GetText() ) {

      std::string str = stEl->GetText();

      std::vector<std::string> pol_list = OPAQ::StringTools::tokenize( str, ",:;| \t", 6 );
      
      for ( auto it = pol_list.begin(); it != pol_list.end(); ++it) {
	
	OPAQ::Pollutant *p =
	  OPAQ::Config::PollutantManager::getInstance()->find(*it);
	if (!p)
	  throw OPAQ::BadConfigurationException( "unknown pollutant found for " + name + " : " + *it);
	
	// add to the pollutants list for this station
	st->getPollutants().push_back(p);
      }

    }      
    _net.getStations().push_back(st);
    
    stEl = stEl->NextSiblingElement("station");
  } /* end while loop over station elements */
  
  if (_net.getStations().size() == 0)
    throw OPAQ::BadConfigurationException("no stations defined in network");
  
}
  
OPAQ::AQNetwork* XMLAQNetProvider::getAQNetwork() {
  return &_net;
}
  
} /* namespace OPAQ */

OPAQ_REGISTER_PLUGIN(OPAQ::XMLAQNetProvider);
