/*
 * ConfigurationHandler.cpp
 *
 *  Created on: Jan 15, 2014
 *      Author: vlooys
 */

#include "ConfigurationHandler.h"

namespace OPAQ
{

ConfigurationHandler::ConfigurationHandler()
: _logger("OPAQ::ConfigurationHandler")
{
}

/* ================================================================================
   Forecast stage parser
   ============================================================================= */
OPAQ::Config::ForecastStage* ConfigurationHandler::parseForecastStage(TiXmlElement* element)
{
    auto fcStage = std::make_unique<Config::ForecastStage>();

    TiXmlElement* modelsElement = element->FirstChildElement("models");
    if (!modelsElement) {
        _logger->error("No models tag given in forecast stage configuration...");
        throw RunTimeException("No models tag given in forecast stage configuration !");
    }

    TiXmlElement* componentElement = modelsElement->FirstChildElement("component");
    while (componentElement)
    {
        std::string componentName = componentElement->GetText();
        fcStage->getModels().push_back(&findComponent(componentName));
        componentElement = componentElement->NextSiblingElement("component");
    }

    // parse the <input> section
    TiXmlElement* inputElement = element->FirstChildElement("input");
    try
    {
        std::string componentName = XmlTools::getText(inputElement, "observations");
        fcStage->setValues(&findComponent(componentName));
    }
    catch (const ElementNotFoundException&)
    {
        throw BadConfigurationException("No observation data provider in run configuration");
    }
    try
    {
        std::string componentName = XmlTools::getText(inputElement, "meteo");
        fcStage->setMeteo(&findComponent(componentName));
    }
    catch (const ElementNotFoundException&)
    {
        fcStage->setMeteo(nullptr);
        _logger->warn("no meteo dataprovider in the run configuration");
    }

    try
    {
        std::string bufferName = XmlTools::getText(element, "buffer");
        fcStage->setBuffer(&findComponent(bufferName));
    }
    catch (const ElementNotFoundException&)
    {
        fcStage->setBuffer(nullptr);
        _logger->warn("no databuffer given in run configuration");
    }

    try
    {
        std::string outputName = XmlTools::getText(element, "output");
        fcStage->setOutputWriter(&findComponent(outputName));
    }
    catch (const ElementNotFoundException&)
    {
        fcStage->setOutputWriter(nullptr);
        _logger->warn("no output writer given in run configuration");
    }

    try
    {
        int fc_hor_max = atoi(XmlTools::getText(element, "horizon").c_str());
        TimeInterval fc_hor(fc_hor_max, TimeInterval::Days);
        fcStage->setHorizon(fc_hor);
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("no forecast <horizon> given in forecast run configuration, using default 2");
        fcStage->setHorizon(TimeInterval(2, TimeInterval::Days));
    }

    return fcStage.release();
}

/* ================================================================================
   Mapping stage parser
   ============================================================================= */
OPAQ::Config::MappingStage* ConfigurationHandler::parseMappingStage(TiXmlElement* element)
{

    OPAQ::Config::MappingStage* mapStage = nullptr;

    return mapStage;
}

/* ===========================================================================
 This is the main configuration file parser
 ======================================================================== */
void ConfigurationHandler::parseConfigurationFile(std::string& filename, Config::PollutantManager& pollutantMgr)
{

    clearConfig();

    _doc = TiXmlDocument(filename);
    if (!_doc.LoadFile(filename)) {
        throw BadConfigurationException("Unable to load configuration file : " + filename);
    }
    TiXmlElement* rootElement;
    rootElement = _doc.FirstChildElement("opaq");
    if (!rootElement) {
        throw BadConfigurationException("Unable to find opaq tag in configuration file : " + filename);
    }

    /* ------------------------------------------------------------------------
     First we parse what is available to the OPAQ system in general,
     irrespective of the actual run requested. This defines what is available
     in the OPAQ configuration
     --------------------------------------------------------------------- */

    // Parsing plugins section
    TiXmlDocument pluginsDoc;
    TiXmlElement* pluginsElement = XmlTools::getElement(rootElement, "plugins", &pluginsDoc);
    std::string pluginPath       = XmlTools::getText(pluginsElement, "path");

    // adjusting this to make use of attributes, the config looks much cleaner this way...
    TiXmlElement* pluginElement = pluginsElement->FirstChildElement("plugin");
    while (pluginElement)
    {
        Config::Plugin plugin;
        plugin.setName(pluginElement->Attribute("name"));
        std::string fullname = pluginElement->GetText();
        fullname             = pluginPath + "/" + fullname;
        plugin.setLib(fullname);
        _opaqRun.getPlugins().push_back(plugin);
        pluginElement = pluginElement->NextSiblingElement("plugin");
    }

    // Parsing components section
    TiXmlDocument componentsDoc;
    TiXmlElement* componentsElement = XmlTools::getElement(rootElement, "components", &componentsDoc);
    TiXmlElement* componentElement  = componentsElement->FirstChildElement("component");
    while (componentElement)
    {
        Config::Component component;
        component.setName(componentElement->Attribute("name"));
        std::string pluginName = componentElement->Attribute("plugin");
        component.setPlugin(&findPlugin(pluginName));
        _configDocs.push_back(std::make_unique<TiXmlDocument>());
        component.setConfig(XmlTools::getElement(componentElement, "config", _configDocs.back().get()));
        _opaqRun.getComponents().push_back(component);
        componentElement = componentElement->NextSiblingElement("component");
    }

    // Parsing pollutants section
    try
    {
        TiXmlDocument pollutantsDoc;
        TiXmlElement* pollutantsElement = XmlTools::getElement(rootElement, "pollutants", &pollutantsDoc);
        pollutantMgr.configure(pollutantsElement);
    }
    catch (ElementNotFoundException& e)
    {
        std::stringstream ss;
        ss << "no pollutants section in configuration file: " << e.what();
        _logger->critical(ss.str());
        exit(1);
    }
    if (pollutantMgr.getList().size() == 0) {
        _logger->critical("pollutant list is empty: define at least 1 pollutant");
        exit(1);
    }

    _logger->info("Pollutant list:");
    std::vector<Pollutant>* pols        = &(pollutantMgr.getList());
    std::vector<Pollutant>::iterator it = pols->begin();
    while (it != pols->end())
    {
        _logger->info(" " + (*it++).toString());
    }

    /* ------------------------------------------------------------------------
     Now we parse the run information, which defines how OPAQ should be run
     i.e. for what pollutant and what timesteps we should do ? Also this
     defines the forecast/mapping stages in the OPAQ run...

     Let's us the rootElement pointer for this again...
     --------------------------------------------------------------------- */
    TiXmlDocument runConfigDoc;
    try
    {
        rootElement = XmlTools::getElement(rootElement, "runconfig", &runConfigDoc);
    }
    catch (ElementNotFoundException& e)
    {
        std::stringstream ss;
        ss << "no runconfig in configuration file: " << e.what();
        _logger->critical(ss.str());
        exit(1);
    }

    /* ------------------------------------------------------------------------
     Parsing base times section
     --------------------------------------------------------------------- */
    try
    {
        TiXmlElement* basetimesElement = XmlTools::getElement(rootElement, "basetimes");
        TiXmlElement* basetimeElement  = basetimesElement->FirstChildElement("basetime");
        while (basetimeElement)
        {
            std::string timestamp = std::string(basetimeElement->GetText());
            try
            {
                _opaqRun.getBaseTimes().push_back(DateTimeTools::parseDateTime(timestamp));
            }
            catch (ParseException& e)
            {
                _logger->critical("Failed to parse base time: " + timestamp);
                _logger->critical(e.what());
                exit(1);
            }
            basetimeElement = basetimeElement->NextSiblingElement("basetime");
        }
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("no base times section in configuration file"); // but might be given using command line args
    }

    /* ------------------------------------------------------------------------
     Parsing pollutant elements section
     --------------------------------------------------------------------- */
    try
    {
        std::string name = XmlTools::getText(rootElement, "pollutant");
        _opaqRun.setPollutantName(name); // set the pollutant name
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("no pollutant set in configuration file"); // but might be given using command line args
    }

    try
    {
        std::string name = XmlTools::getText(rootElement, "aggregation");
        _opaqRun.setAggregation(name); // set the aggregation
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("no aggregation set in configuration file"); // but might be given using command line args
    }

    /* ------------------------------------------------------------------------
     Parsing network section : selects the component which will deliver the
     AQ network configuration
     --------------------------------------------------------------------- */
    try
    {
        TiXmlElement* networkElement = XmlTools::getElement(rootElement, "network");
        std::string componentName    = XmlTools::getText(networkElement, "component");
        _opaqRun.setNetworkProvider(&findComponent(componentName));
    }
    catch (const ElementNotFoundException& e)
    {
        _logger->critical("no air quality network defined");
        _logger->critical(e.what());
        exit(1);
    }

    /* ------------------------------------------------------------------------
     Parsing grid section : selects the component which will deliver the
     Grid configuration
     --------------------------------------------------------------------- */
    try
    {
        TiXmlElement* gridElement = XmlTools::getElement(rootElement, "grid");
        std::string componentName = XmlTools::getText(gridElement, "component");
        _opaqRun.setGridProvider(&findComponent(componentName));
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("no grid provider defined");
        _opaqRun.setGridProvider(nullptr);
    }

    /* ------------------------------------------------------------------------
     Parsing forecast section
     --------------------------------------------------------------------- */
    try
    {
        TiXmlElement* forecastElement        = XmlTools::getElement(rootElement, "forecast");
        Config::ForecastStage* forecastStage = parseForecastStage(forecastElement);
        _opaqRun.setForecastStage(forecastStage);
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("no forecast stage defined");
        _opaqRun.setForecastStage(nullptr);
    }

    /* ------------------------------------------------------------------------
     Parsing mapping section
     --------------------------------------------------------------------- */
    try
    {
        TiXmlElement* mappingElement       = XmlTools::getElement(rootElement, "mapping");
        Config::MappingStage* mappingStage = parseMappingStage(mappingElement);
        _opaqRun.setMappingStage(mappingStage);
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("no mapping stage defined");
        _opaqRun.setMappingStage(nullptr);
    }
}

void ConfigurationHandler::validateConfiguration(Config::PollutantManager& pollutantMgr)
{
    // check for plugins with the same name
    for (auto it1 = _opaqRun.getPlugins().begin(); it1 != _opaqRun.getPlugins().end(); ++it1)
    {
        std::string name = it1->getName();
        for (auto it2 = it1 + 1; it2 != _opaqRun.getPlugins().end(); ++it2)
        {
            if (name == it2->getName())
            {
                throw BadConfigurationException("Found 2 plugins with the same name: " + name);
            }
        }

        // check if plugin lib file exists
        if (!FileTools::exists(it1->getLib()))
        {
            throw BadConfigurationException("Library file not found: " + it1->getLib());
        }
    }

    // check for components with the same name
    for (auto it1 = _opaqRun.getComponents().begin(); it1 != _opaqRun.getComponents().end(); ++it1) 
    {
        std::string name = it1->getName();
        for (auto it2 = it1 + 1; it2 != _opaqRun.getComponents().end(); ++it2)
        {
            if (name == it2->getName())
            {
                throw BadConfigurationException("Found 2 components with the same name: " + name);
            }
        }
    }

    // check if base times are defined
    if (_opaqRun.getBaseTimes().empty())
    {
        throw BadConfigurationException("No base times defined (need at least 1)");
    }

    // check if pollutant is defined
    if (!_opaqRun.pollutantIsSet())
    {
        throw BadConfigurationException("No pollutant set");
    }

    if (!pollutantMgr.find(_opaqRun.getPollutantName()))
    {
        throw BadConfigurationException("pollutant '" + _opaqRun.getPollutantName() + "' not found in pollutant list");
    }
}

void ConfigurationHandler::clearConfig()
{
    _opaqRun.getPlugins().clear();
    _opaqRun.getComponents().clear();
    _opaqRun.getBaseTimes().clear();
    //_opaqRun.setPollutant(nullptr);
    _opaqRun.setForecastStage(nullptr);
    _opaqRun.setMappingStage(nullptr);
}

OPAQ::Config::Plugin& ConfigurationHandler::findPlugin(const std::string& pluginName)
{
    auto iter = std::find_if(_opaqRun.getPlugins().begin(), _opaqRun.getPlugins().end(), [&](Config::Plugin& plugin) {
        return plugin.getName() == pluginName;
    });

    if (iter == _opaqRun.getPlugins().end())
    {
        _logger->error("Plugin with name '{}' not found.", pluginName);
        throw BadConfigurationException("Plugin with name '" + pluginName + "' not found.");
    }

    return *iter;
}

OPAQ::Config::Component& ConfigurationHandler::findComponent(const std::string& componentName)
{
    auto iter = std::find_if(_opaqRun.getComponents().begin(), _opaqRun.getComponents().end(), [&](Config::Component& comp) {
        return comp.getName() == componentName;
    });

    if (iter == _opaqRun.getComponents().end())
    {
        _logger->error("Plugin with name '{}' not found.", componentName);
        throw BadConfigurationException("Component with name '" + componentName + "' not found.");
    }

    return *iter;
}

} /* namespace OPAQ */
