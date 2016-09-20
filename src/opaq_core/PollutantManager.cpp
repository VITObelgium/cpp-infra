#include "PollutantManager.h"

namespace OPAQ {

namespace Config {

PollutantManager::PollutantManager()
: logger("OPAQ::Config::PollutantManager") {
}

PollutantManager::~PollutantManager() {
}

PollutantManager *PollutantManager::getInstance() {
	static PollutantManager instance;
	return &instance;
}

OPAQ::Pollutant *PollutantManager::find(std::string name) {

	for (std::vector<OPAQ::Pollutant>::iterator it = pollutants.begin();
			it != pollutants.end(); it++) {
		OPAQ::Pollutant *p = &(*it);
		if (p->getName() == name)
			return p;
	}
	logger->warn("Pollutant with name '" + name + "' not found.");
	return NULL;
}

void PollutantManager::configure(TiXmlElement const * config) {
	pollutants.clear();
	const TiXmlElement * pollutantElement = config->FirstChildElement("pollutant");
	while (pollutantElement) {
		OPAQ::Pollutant pol(pollutantElement);
		pollutants.push_back(pol);
		pollutantElement = pollutantElement->NextSiblingElement("pollutant");
	}
}

std::ostream& operator<<(std::ostream& os, const PollutantManager& s) {
	for (std::vector<OPAQ::Pollutant>::const_iterator it = s.pollutants.begin();
			it != s.pollutants.end(); it++)
		os << (*it) << std::endl;
	return os;
}

}

}
