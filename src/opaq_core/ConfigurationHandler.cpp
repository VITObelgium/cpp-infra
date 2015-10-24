/*
 * ConfigurationHandler.cpp
 *
 *  Created on: Jan 15, 2014
 *      Author: vlooys
 */

#include "ConfigurationHandler.h"

namespace OPAQ {

LOGGER_DEF(OPAQ::ConfigurationHandler);

OPAQ::Config::Stage* ConfigurationHandler::parseStage(TiXmlElement * element) {

	OPAQ::Config::Stage *stage;
	TiXmlElement *modelElement = element->FirstChildElement("model");

	if (modelElement) {

		// singleton model
		stage = new Config::Singleton(); // deleted in OpaqRun destructor
		Config::Singleton* singleton = (Config::Singleton *) stage;
		std::string componentName = XmlTools::getText(modelElement, "component");
		singleton->setModel(findComponent(componentName));
		TiXmlElement * inputElement = modelElement->FirstChildElement("input");
		try {
			componentName = XmlTools::getText(inputElement, "values");
			singleton->setValues(findComponent(componentName));
		} catch (ElementNotFoundException & e) {
			singleton->setValues(NULL);
		}
		try {
			componentName = XmlTools::getText(inputElement, "meteo");
			singleton->setMeteo(findComponent(componentName));
		} catch (ElementNotFoundException & e) {
			singleton->setMeteo(NULL);
		}
		try {
			componentName = XmlTools::getText(inputElement, "historical_forecasts");
			singleton->setHistoricalForecasts(findComponent(componentName));
		} catch (ElementNotFoundException & e) {
			singleton->setHistoricalForecasts(NULL);
		}
		componentName = XmlTools::getText(modelElement, "output");
		singleton->setOutput(findComponent(componentName));

	} else {

		// ensemble
		TiXmlElement * ensembleElement = element->FirstChildElement("ensemble");
		if (!ensembleElement) {
			logger->error("No valid model type found ('model' or 'ensemble')");
			throw RunTimeException ("No valid model type found ('model' or 'ensemble')");
		}

		stage = new Config::Ensemble(); // deleted in OpaqRun destructor
		Config::Ensemble * ensemble = (Config::Ensemble *) stage;
		TiXmlElement * modelsElement = XmlTools::getElement(ensembleElement, "models");

		// TODO: does tinyxml preserve the order of the child elements ??
		TiXmlElement * componentElement = modelsElement->FirstChildElement(
				"component");
		while (componentElement) {
			std::string componentName = componentElement->GetText();
			ensemble->getModels().push_back(findComponent(componentName));
			componentElement = componentElement->NextSiblingElement(
					"component");
		}

		TiXmlElement * inputElement = modelElement->FirstChildElement("input");
		try {
			std::string componentName = XmlTools::getText(inputElement, "values");
			ensemble->setValues(findComponent(componentName));
		} catch (ElementNotFoundException & e) {
			ensemble->setValues(NULL);
		}
		try {
			std::string componentName = XmlTools::getText(inputElement, "meteo");
			ensemble->setMeteo(findComponent(componentName));
		} catch (ElementNotFoundException & e) {
			ensemble->setMeteo(NULL);
		}
		try {
			std::string componentName = XmlTools::getText(inputElement, "historical_forecasts");
			ensemble->setHistoricalForecasts(findComponent(componentName));
		} catch (ElementNotFoundException & e) {
			ensemble->setHistoricalForecasts(NULL);
		}

		std::string mergerName = XmlTools::getText(ensembleElement, "merger");
		ensemble->setMerger(findComponent(mergerName));

		std::string outputName = XmlTools::getText(ensembleElement, "output");
		ensemble->setOutput(findComponent(outputName));
	}
	return stage;
}

/* ===========================================================================
 This is the main configuration file parser
 ======================================================================== */
void ConfigurationHandler::parseConfigurationFile(std::string & filename) {

	clearConfig();

	doc = TiXmlDocument(filename);
	if (!doc.LoadFile(filename)) {
		logger->fatal("Failed to load configuration file");
		exit(1);
	}
	TiXmlElement * rootElement;
	rootElement = doc.FirstChildElement("opaq");
	if (!rootElement) {
		logger->fatal("Root element ('opaq') not found");
		exit(1);
	}

	/* ------------------------------------------------------------------------
	 First we parse what is available to the OPAQ system in general,
	 irrespective of the actual run requested. This defines what is available
	 in the OPAQ configuration
	 --------------------------------------------------------------------- */

	// Parsing plugins section
	TiXmlDocument pluginsDoc;
	TiXmlElement * pluginsElement = XmlTools::getElement(rootElement, "plugins", &pluginsDoc);
	std::string pluginPath = XmlTools::getText( pluginsElement, "pluginpath" );

	TiXmlElement * pluginElement = pluginsElement->FirstChildElement("plugin");
	while (pluginElement) {
		Config::Plugin plugin;
		plugin.setName(XmlTools::getText(pluginElement, "name"));
	
		std::string fullname = pluginPath + "/" + XmlTools::getText(pluginElement, "lib");
		plugin.setLib( fullname );

		logger->info( "Loading " + fullname );
		opaqRun.getPlugins().push_back(plugin);
		pluginElement = pluginElement->NextSiblingElement("plugin");
	}

	// Parsing components section
	TiXmlDocument componentsDoc;
	TiXmlElement * componentsElement = XmlTools::getElement(rootElement, "components", &componentsDoc);
	TiXmlElement * componentElement = componentsElement->FirstChildElement(
			"component");
	while (componentElement) {
		Config::Component component;
		component.setName(XmlTools::getText(componentElement, "name"));
		std::string pluginName = XmlTools::getText(componentElement, "plugin");
		component.setPlugin(findPlugin(pluginName));
		TiXmlDocument * configDoc = new TiXmlDocument();
		_configDocs.push_back(configDoc);
		component.setConfig(XmlTools::getElement(componentElement, "config", configDoc));
		opaqRun.getComponents().push_back(component);
		componentElement = componentElement->NextSiblingElement("component");
	}

	// Parsing pollutants section
	try {
		TiXmlDocument pollutantsDoc;
		TiXmlElement * pollutantsElement = XmlTools::getElement(rootElement, "pollutants", &pollutantsDoc);
		Config::PollutantManager::getInstance()->configure(pollutantsElement);
	} catch (ElementNotFoundException & e) {
		std::stringstream ss;
		ss << "no pollutants section in configuration file: " << e.what();
		logger->fatal(ss.str());
		exit(1);
	}
	if (Config::PollutantManager::getInstance()->getList().size() == 0) {
		logger->fatal("pollutant list is empty: define at least 1 pollutant");
		exit(1);
	}

	logger->info("Pollutant list:");
	std::vector<Pollutant> * pols = &(Config::PollutantManager::getInstance()->getList());
	std::vector<Pollutant>::iterator it = pols->begin();
	while (it != pols->end()) {
		logger->info(" " + (*it++).toString());
	}

//	std::cout << "Pollutant list : " << std::endl;
//	std::cout << *Config::PollutantManager::getInstance();

	/* ------------------------------------------------------------------------
	 Now we parse the run information, which defines how OPAQ should be run
	 i.e. for what pollutant and what timesteps we should do ? Also this
	 defines the forecast/mapping stages in the OPAQ run...

	 Let's us the rootElement pointer for this again...
	 --------------------------------------------------------------------- */
	TiXmlDocument runConfigDoc;
	try {
		rootElement = XmlTools::getElement(rootElement, "runconfig", &runConfigDoc);
	} catch (ElementNotFoundException & e) {
		std::stringstream ss;
		ss << "no runconfig in configuration file: " << e.what();
		logger->fatal(ss.str());
		exit(1);
	}

	/* ------------------------------------------------------------------------
	 Parsing base times section
	 --------------------------------------------------------------------- */
	try {
		TiXmlElement * basetimesElement = XmlTools::getElement(rootElement, "basetimes");
		TiXmlElement * basetimeElement = basetimesElement->FirstChildElement(
				"basetime");
		while (basetimeElement) {
			std::string timestamp = std::string(basetimeElement->GetText());
			try {
				opaqRun.getBaseTimes().push_back(
						DateTimeTools::parseDateTime(timestamp));
			} catch (ParseException & e) {
				logger->fatal("Failed to parse base time: " + timestamp);
				logger->fatal(e.what());
				exit(1);
			}
			basetimeElement = basetimeElement->NextSiblingElement("basetime");
		}
	} catch (ElementNotFoundException & e) {
		logger->warn("no base times section in configuration file");	// but might be given using command line args
	}

	/* ------------------------------------------------------------------------
	 Parsing pollutant elements section
	 --------------------------------------------------------------------- */
	try {
		std::string name = XmlTools::getText(rootElement, "pollutant");
		opaqRun.setPollutantName(name); // set the pollutant name
	} catch (ElementNotFoundException & e) {
		logger->warn("no pollutant set in configuration file");		// but might be given using command line args
	}

	/* ------------------------------------------------------------------------
	 Parsing network section : selects the component which will deliver the
	 AQ network configuration
	 --------------------------------------------------------------------- */
	try {
		TiXmlElement *networkElement = XmlTools::getElement(rootElement, "network");
		std::string componentName = XmlTools::getText(networkElement, "component");
		opaqRun.setNetworkProvider(findComponent(componentName));
	} catch (ElementNotFoundException & e) {
		logger->fatal("no air quality network defined");
		logger->fatal(e.what());
		exit(1);
	}

	/* ------------------------------------------------------------------------
	 Parsing grid section : selects the component which will deliver the
	 Grid configuration
	 --------------------------------------------------------------------- */

	try {
		TiXmlElement *gridElement = XmlTools::getElement(rootElement, "grid");
		std::string componentName = XmlTools::getText(gridElement, "component");
		opaqRun.setGridProvider(findComponent(componentName));
	} catch (ElementNotFoundException & e) {
		logger->warn("no grid provider defined");
		opaqRun.setGridProvider(NULL);
	}

	/* ------------------------------------------------------------------------
	 Parsing forecast section
	 --------------------------------------------------------------------- */
	try {
		TiXmlElement * forecastElement = XmlTools::getElement(rootElement, "forecast");
		Config::Stage * forecastStage = parseStage(forecastElement);
		opaqRun.setForecastStage(forecastStage);
	} catch (ElementNotFoundException & e) {
		logger->warn("no forecast stage defined");
		opaqRun.setForecastStage(NULL);
	}

	/* ------------------------------------------------------------------------
	 Parsing mapping section
	 --------------------------------------------------------------------- */
	try {
		TiXmlElement * mappingElement = XmlTools::getElement(rootElement, "mapping");
		Config::Stage * mappingStage = parseStage(mappingElement);
		opaqRun.setMappingStage(mappingStage);
	} catch (ElementNotFoundException & e) {
		logger->warn("no mapping stage defined");
		opaqRun.setMappingStage(NULL);
	}

}

void ConfigurationHandler::validateConfiguration() {

	// check for plugins with the same name
	for (std::vector<OPAQ::Config::Plugin>::iterator it1 =
			opaqRun.getPlugins().begin(); it1 != opaqRun.getPlugins().end();
			it1++) {
		std::string name = (*it1).getName();
		for (std::vector<OPAQ::Config::Plugin>::iterator it2 =
				opaqRun.getPlugins().begin(); it2 != opaqRun.getPlugins().end();
				it2++) {
			if (it1 != it2 && name.compare((*it2).getName()) == 0) {
				logger->error("Found 2 plugins with the same name: " + name);
				exit(1);
			}
		}

		// check if plugin lib file exists
		std::string lib = (*it1).getLib();
		if (!FileTools::exists(lib)) {
			logger->fatal("Library file not found: " + lib);
			exit(1);
		}
	}

	// check for components with the same name
	for (std::vector<OPAQ::Config::Component>::iterator it1 =
			opaqRun.getComponents().begin();
			it1 != opaqRun.getComponents().end(); it1++) {
		std::string name = (*it1).getName();
		for (std::vector<OPAQ::Config::Component>::iterator it2 =
				opaqRun.getComponents().begin();
				it2 != opaqRun.getComponents().end(); it2++) {
			if (it1 != it2 && name.compare((*it2).getName()) == 0) {
				logger->error("Found 2 components with the same name: " + name);
				exit(1);
			}
		}
	}

	// check if base times are defined
	if (opaqRun.getBaseTimes().size() == 0) {
		logger->error("no base times defined (need at least 1)");
		exit(1);
	}

	// check if pollutant is defined
	if (!opaqRun.pollutantIsSet()) {
		logger->error("no pollutant set");
		exit(1);
	}
	std::string pollutantName = opaqRun.getPollutantName();
	Config::PollutantManager * pm = Config::PollutantManager::getInstance();
	Pollutant * pol = pm->find(pollutantName);
	if (pol == NULL) {
		logger->error("pollutant '" + pollutantName + "' not found in pollutant list");
		exit(1);
	}
}

void ConfigurationHandler::clearConfig() {
	opaqRun.getPlugins().clear();
	opaqRun.getComponents().clear();
	opaqRun.getBaseTimes().clear();
	//opaqRun.setPollutant(NULL);
	opaqRun.setForecastStage(NULL);
	opaqRun.setMappingStage(NULL);
}

OPAQ::Config::Plugin * ConfigurationHandler::findPlugin(
		std::string & pluginName) {
	for (std::vector<OPAQ::Config::Plugin>::iterator it =
			opaqRun.getPlugins().begin(); it != opaqRun.getPlugins().end();
			it++) {
		OPAQ::Config::Plugin * plugin = &(*it);
		if (plugin->getName() == pluginName) {
			return plugin;
		}
	}
	logger->error("Plugin with name '" + pluginName + "' not found.");
	exit(1);
	return NULL;
}

OPAQ::Config::Component * ConfigurationHandler::findComponent(
		std::string & componentName) {
	for (std::vector<OPAQ::Config::Component>::iterator it =
			opaqRun.getComponents().begin();
			it != opaqRun.getComponents().end(); it++) {
		OPAQ::Config::Component * component = &(*it);
		if (component->getName() == componentName) {
			return component;
		}
	}
	logger->error("Component with name '" + componentName + "' not found.");
	exit(1);
	return NULL;
}

} /* namespace OPAQ */
