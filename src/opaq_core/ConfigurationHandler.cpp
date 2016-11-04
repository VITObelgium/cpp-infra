/*
 * ConfigurationHandler.cpp
 *
 *  Created on: Jan 15, 2014
 *      Author: vlooys
 */

#include "config.h"
#include "ConfigurationHandler.h"
#include "PollutantManager.h"

#include "config/ForecastStage.h"
#include "config/MappingStage.h"

namespace opaq
{

ConfigurationHandler::ConfigurationHandler()
: _logger("ConfigurationHandler")
{
}

config::ForecastStage ConfigurationHandler::parseForecastStage(TiXmlElement* element)
{
    config::ForecastStage fcStage;

    auto* modelsElement = element->FirstChildElement("models");
    if (!modelsElement)
    {
        throw RunTimeException("No models tag given in forecast stage configuration!");
    }

    auto* componentElement = modelsElement->FirstChildElement("component");
    while (componentElement)
    {
        fcStage.addModel(_opaqRun.getComponent(componentElement->GetText()));
        componentElement = componentElement->NextSiblingElement("component");
    }

    // parse the <input> section
    auto* inputElement = element->FirstChildElement("input");
    try
    {
        fcStage.setValues(_opaqRun.getComponent(XmlTools::getText(inputElement, "observations")));
    }
    catch (const ElementNotFoundException&)
    {
        throw BadConfigurationException("No observation data provider in run configuration");
    }

    try
    {
        fcStage.setMeteo(_opaqRun.getComponent(XmlTools::getText(inputElement, "meteo")));
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("No meteo dataprovider in the run configuration");
    }

    try
    {
        fcStage.setBuffer(_opaqRun.getComponent(XmlTools::getText(element, "buffer")));
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("No databuffer given in run configuration");
    }

    try
    {
        fcStage.setOutputWriter(_opaqRun.getComponent(XmlTools::getText(element, "output")));
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("No output writer given in run configuration");
    }

    try
    {
        auto fc_hor_max = atoi(XmlTools::getText(element, "horizon").c_str());
        fcStage.setHorizon(chrono::days(fc_hor_max));
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("no forecast <horizon> given in forecast run configuration, using default 2");
        fcStage.setHorizon(chrono::days(2));
    }

    return fcStage;
}

config::MappingStage ConfigurationHandler::parseMappingStage(TiXmlElement* element)
{
    config::Component stations;
    std::vector<config::Component> models;

    auto* modelsElement = element->FirstChildElement("models");
    if (!modelsElement)
    {
        throw RunTimeException("No models tag given in mapping stage configuration!");
    }

    auto* componentElement = modelsElement->FirstChildElement("component");
    while (componentElement)
    {
        models.push_back(_opaqRun.getComponent(componentElement->GetText()));
        componentElement = componentElement->NextSiblingElement("component");
    }

    // parse the <input> section
    auto* inputElement = element->FirstChildElement("input");
    try
    {
        stations = _opaqRun.getComponent(XmlTools::getText(inputElement, "stations"));
    }
    catch (const ElementNotFoundException&)
    {
        throw BadConfigurationException("No station data provider in run configuration");
    }

    return config::MappingStage(stations, std::move(models));
}

void ConfigurationHandler::parseConfigurationFile(const std::string& filename, config::PollutantManager& pollutantMgr)
{
    _opaqRun = config::OpaqRun();

    _doc = TiXmlDocument(filename);
    if (!_doc.LoadFile(filename))
    {
        throw BadConfigurationException("Unable to load configuration file: {}", filename);
    }
    
    auto* rootElement = _doc.FirstChildElement("opaq");
    if (!rootElement)
    {
        throw BadConfigurationException("Unable to find opaq tag in configuration file: {}", filename);
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
        auto fullname = fmt::format("{}/{}" PLUGIN_EXT, pluginPath, pluginElement->GetText());

        config::Plugin plugin;
        plugin.name    = pluginElement->Attribute("name");
        plugin.libPath = fullname;
        _opaqRun.addPlugin(plugin);
        pluginElement = pluginElement->NextSiblingElement("plugin");
    }

    // Parsing components section
    TiXmlDocument componentsDoc;
    TiXmlElement* componentsElement = XmlTools::getElement(rootElement, "components", &componentsDoc);
    TiXmlElement* componentElement  = componentsElement->FirstChildElement("component");
    while (componentElement)
    {
        _configDocs.push_back(std::make_unique<TiXmlDocument>());

        config::Component component;
        component.name = componentElement->Attribute("name");
        component.plugin = _opaqRun.getPlugin(componentElement->Attribute("plugin"));
        component.config = XmlTools::getElement(componentElement, "config", _configDocs.back().get());
        _opaqRun.addComponent(component);

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
        throw BadConfigurationException("no pollutants section in configuration file: {}", e.what());
    }

    if (pollutantMgr.getList().empty())
    {
        throw BadConfigurationException("pollutant list is empty: define at least 1 pollutant");
    }

    _logger->info("Pollutant list:");
    for (auto& pol : pollutantMgr.getList())
    {
        _logger->info(" {}", pol.toString());
    }

    /* ------------------------------------------------------------------------
     Now we parse the run information, which defines how OPAQ should be run
     i.e. for what pollutant and what timesteps we should do ? Also this
     defines the forecast/mapping stages in the OPAQ run...

     Let's us the rootElement pointer for this again...
     --------------------------------------------------------------------- */
    try
    {
        TiXmlDocument runConfigDoc;
        rootElement = XmlTools::getElement(rootElement, "runconfig", &runConfigDoc);
    }
    catch (ElementNotFoundException& e)
    {
        throw BadConfigurationException("No runconfig in configuration file: {}", e.what());
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
            _opaqRun.addBaseTime(chrono::from_date_time_string(basetimeElement->GetText()));
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
        _opaqRun.setNetworkProvider(_opaqRun.getComponent(componentName));
    }
    catch (const ElementNotFoundException& e)
    {
        _logger->critical("No air quality network defined: {}", e.what());
        throw RunTimeException("Invalid air quality network defined");
    }

    /* ------------------------------------------------------------------------
     Parsing grid section : selects the component which will deliver the
     Grid configuration
     --------------------------------------------------------------------- */
    try
    {
        TiXmlElement* gridElement = XmlTools::getElement(rootElement, "grid");
        std::string componentName = XmlTools::getText(gridElement, "component");
        _opaqRun.setGridProvider(_opaqRun.getComponent(componentName));
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("No grid provider defined");
    }

    /* ------------------------------------------------------------------------
     Parsing forecast section
     --------------------------------------------------------------------- */
    try
    {
        _opaqRun.setForecastStage(parseForecastStage(XmlTools::getElement(rootElement, "forecast")));
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("No forecast stage defined");
    }

    /* ------------------------------------------------------------------------
     Parsing mapping section
     --------------------------------------------------------------------- */
    try
    {
        _opaqRun.setMappingStage(parseMappingStage(XmlTools::getElement(rootElement, "mapping")));
    }
    catch (const ElementNotFoundException&)
    {
        _logger->warn("no mapping stage defined");
    }
}

void ConfigurationHandler::validateConfiguration(config::PollutantManager& pollutantMgr)
{
    // check for plugins with the same name
    auto plugins = _opaqRun.getPlugins();

    for (auto it1 = plugins.begin(); it1 != plugins.end(); ++it1)
    {
        auto name = it1->name;
        for (auto it2 = it1 + 1; it2 != plugins.end(); ++it2)
        {
            if (name == it2->name)
            {
                throw BadConfigurationException("Found 2 plugins with the same name: {}", name);
            }
        }

#ifndef STATIC_PLUGINS
        // check if plugin lib file exists
        if (!FileTools::exists(it1->libPath))
        {
            throw BadConfigurationException("Library file not found: {}", it1->libPath);
        }
#endif
    }

    // check for components with the same name
    auto components = _opaqRun.getComponents();
    for (auto it1 = components.begin(); it1 != components.end(); ++it1)
    {
        auto name = it1->name;
        for (auto it2 = it1 + 1; it2 != components.end(); ++it2)
        {
            if (name == it2->name)
            {
                throw BadConfigurationException("Found 2 components with the same name: {}", name);
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

    // throws if not found
    pollutantMgr.find(_opaqRun.getPollutantName());
}

}
