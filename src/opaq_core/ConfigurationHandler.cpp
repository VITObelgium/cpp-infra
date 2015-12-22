/*
 * ConfigurationHandler.cpp
 *
 *  Created on: Jan 15, 2014
 *      Author: vlooys
 */

#include "ConfigurationHandler.h"

namespace OPAQ {

LOGGER_DEF(OPAQ::ConfigurationHandler);

/* ================================================================================
   Forecast stage parser
   ============================================================================= */
OPAQ::Config::ForecastStage* ConfigurationHandler::parseForecastStage(TiXmlElement * element) {

  OPAQ::Config::ForecastStage *fcStage;

  // create a new forecast stage object
  fcStage = new OPAQ::Config::ForecastStage();
  
  TiXmlElement *modelsElement = element->FirstChildElement("models");
  if ( ! modelsElement ) {
    logger->error( "No models tag given in forecast stage configuration..." );
    throw RunTimeException("No models tag given in forecast stage configuration !");
  }

  TiXmlElement * componentElement = modelsElement->FirstChildElement("component");
  while (componentElement) {
    std::string componentName = componentElement->GetText();
    fcStage->getModels().push_back(findComponent(componentName));
    componentElement = componentElement->NextSiblingElement("component");
    }
    
  // parse the <input> section
  TiXmlElement * inputElement = element->FirstChildElement("input");
  try {
    std::string componentName = XmlTools::getText(inputElement, "observations");
    fcStage->setValues(findComponent(componentName));
  } catch (ElementNotFoundException & e) {
    throw BadConfigurationException( "No observation data provider in run configuration" );
  }
  try {
    std::string componentName = XmlTools::getText(inputElement, "meteo");
    fcStage->setMeteo(findComponent(componentName));
  } catch (ElementNotFoundException & e) {
    fcStage->setMeteo(NULL);
    logger->warn( "no meteo dataprovider in the run configuration" );
  }


  try {
    std::string bufferName = XmlTools::getText(element, "buffer");
    fcStage->setBuffer(findComponent(bufferName));
  } catch (ElementNotFoundException & e ) {
    fcStage->setBuffer(NULL);
    logger->warn( "no databuffer given in run configuration" );
  }

  try {
    std::string outputName = XmlTools::getText(element, "output");
    fcStage->setOutputWriter(findComponent(outputName));
  } catch (ElementNotFoundException & e ) {
    fcStage->setOutputWriter(NULL);
    logger->warn( "no output writer given in run configuration" );
  }


  try {
    int fc_hor_max = atoi( XmlTools::getText(element, "horizon").c_str() );
    TimeInterval fc_hor( fc_hor_max, TimeInterval::Days );
    fcStage->setHorizon( fc_hor );
  } catch ( ElementNotFoundException & e ) {
    logger->warn( "no forecast <horizon> given in forecast run configuration, using default 2" );
    fcStage->setHorizon( TimeInterval( 2, TimeInterval::Days ) );
  }

  return fcStage;
}


/* ================================================================================
   Mapping stage parser
   ============================================================================= */
OPAQ::Config::MappingStage* ConfigurationHandler::parseMappingStage(TiXmlElement * element) {

  OPAQ::Config::MappingStage *mapStage = NULL;



  return mapStage;
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
    opaqRun.getPlugins().push_back(plugin);
    pluginElement = pluginElement->NextSiblingElement("plugin");
  }
  
  // Parsing components section
  TiXmlDocument componentsDoc;
  TiXmlElement * componentsElement = XmlTools::getElement(rootElement, "components", &componentsDoc);
  TiXmlElement * componentElement = componentsElement->FirstChildElement("component");
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
    TiXmlElement * basetimeElement = basetimesElement->FirstChildElement("basetime");
    while (basetimeElement) {
      std::string timestamp = std::string(basetimeElement->GetText());
      try {
	opaqRun.getBaseTimes().push_back(DateTimeTools::parseDateTime(timestamp));
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
    Config::ForecastStage *forecastStage = parseForecastStage( forecastElement );
    opaqRun.setForecastStage( forecastStage);
  } catch (ElementNotFoundException & e) {
    logger->warn("no forecast stage defined");
    opaqRun.setForecastStage(NULL);
  }
  
  /* ------------------------------------------------------------------------
     Parsing mapping section
     --------------------------------------------------------------------- */
  try {
    TiXmlElement * mappingElement = XmlTools::getElement(rootElement, "mapping");
    Config::MappingStage * mappingStage = parseMappingStage(mappingElement);
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

OPAQ::Config::Plugin * ConfigurationHandler::findPlugin( std::string & pluginName ) {
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
  
OPAQ::Config::Component * ConfigurationHandler::findComponent(std::string & componentName) {
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
