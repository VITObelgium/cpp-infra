#include "ConfigurationHandler.h"
#include "GridType.h"
#include "PollutantManager.h"
#include "config.h"

#include "config/ForecastStage.h"
#include "config/MappingStage.h"
#include "tools/XmlTools.h"

namespace opaq {

using namespace infra;

ConfigurationHandler::ConfigurationHandler()
: _logger("ConfigurationHandler")
{
}

static std::string getChildElement(const ConfigNode& node, const char* name)
{
    auto val = node.child("name").value();
    if (val.empty()) {
        throw BadConfigurationException("Node '{}' not found in run configuration");
    }

    return std::string(val);
}

config::ForecastStage ConfigurationHandler::parseForecastStage(const ConfigNode& element)
{
    boost::optional<config::Component> meteo;
    std::vector<config::Component> models;

    auto modelsElement = element.child("models");
    if (!modelsElement) {
        throw RunTimeException("No models tag given in forecast stage configuration!");
    }

    for (auto& componentElement : modelsElement.children("component")) {
        models.push_back(_opaqRun.getComponent(std::string(componentElement.value())));
    }

    // parse the <input> section
    auto inputElement = element.child("input");
    auto values       = _opaqRun.getComponent(getChildElement(inputElement, "observations"));
    auto buffer       = _opaqRun.getComponent(getChildElement(inputElement, "meteo"));
    auto outputWriter = _opaqRun.getComponent(getChildElement(inputElement, "output"));

    auto fcHor = chrono::days(element.child("horizon").value<int>().value_or(2));
    return config::ForecastStage(fcHor, values, buffer, outputWriter, meteo, models);
}

config::MappingStage ConfigurationHandler::parseMappingStage(const ConfigNode& element)
{
    std::vector<config::Component> models;

    auto gridType = gridTypeFromString(std::string(element.child("gridtype").value()));

    auto modelsElement = element.child("models");
    if (!modelsElement) {
        throw RunTimeException("No models tag given in mapping stage configuration!");
    }

    for (auto& componentElement : modelsElement.children("component")) {
        models.push_back(_opaqRun.getComponent(std::string(componentElement.value())));
    }

    // parse the <input> section
    // parse the <input> section
    auto inputElement = element.child("input");
    auto observations = _opaqRun.getComponent(getChildElement(inputElement, "observations"));
    auto buffer       = _opaqRun.getComponent(getChildElement(inputElement, "buffer"));

    return config::MappingStage(gridType, observations, buffer, std::move(models));
}

void ConfigurationHandler::parseConfigurationFile(const std::string& filename, config::PollutantManager& pollutantMgr)
{
    _opaqRun = config::OpaqRun();

    try {
        _doc = ConfigDocument::loadFromFile(filename);

        auto rootElement = _doc.child("opaq");
        if (!rootElement) {
            throw BadConfigurationException("Unable to find opaq tag in configuration file: {}", filename);
        }

        /* ------------------------------------------------------------------------
         First we parse what is available to the OPAQ system in general,
         irrespective of the actual run requested. This defines what is available
         in the OPAQ configuration
         --------------------------------------------------------------------- */

        // Parsing plugins section
        ConfigDocument pluginsDoc;
        auto pluginsElement = XmlTools::getElement(rootElement, "plugins", &pluginsDoc);
        auto pluginPath     = pluginsElement.trimmedValue();

        // adjusting this to make use of attributes, the config looks much cleaner this way...
        for (auto& pluginElement : pluginsElement.children("plugin")) {
            auto fullname = fmt::format("{}/{}" PLUGIN_EXT, pluginPath, pluginElement.value());

            config::Plugin plugin;
            plugin.name    = pluginElement.attribute("name");
            plugin.libPath = fullname;
            _opaqRun.addPlugin(plugin);
        }

        // Parsing components section
        ConfigDocument componentsDoc;
        auto componentsElement = XmlTools::getElement(rootElement, "components", &componentsDoc);
        for (auto& componentElement : componentsElement.children("component")) {
            _configDocs.push_back(ConfigDocument());

            config::Component component;
            component.name   = componentElement.attribute("name");
            component.plugin = _opaqRun.getPlugin(std::string(componentElement.attribute("plugin")));
            component.config = XmlTools::getElement(componentElement, "config", &_configDocs.back());
            _opaqRun.addComponent(component);
        }

        // Parsing pollutants section
        ConfigDocument pollutantsDoc;
        auto pollutantsElement = XmlTools::getElement(rootElement, "pollutants", &pollutantsDoc);
        if (pollutantsElement) {
            pollutantMgr.configure(pollutantsElement);
        } else {
            throw BadConfigurationException("no pollutants section in configuration file");
        }

        if (pollutantMgr.getList().empty()) {
            throw BadConfigurationException("pollutant list is empty: define at least 1 pollutant");
        }

        _logger->info("Pollutant list:");
        for (auto& pol : pollutantMgr.getList()) {
            _logger->info(" {}", pol.toString());
        }

        /* ------------------------------------------------------------------------
         Now we parse the run information, which defines how OPAQ should be run
         i.e. for what pollutant and what timesteps we should do ? Also this
         defines the forecast/mapping stages in the OPAQ run...
         --------------------------------------------------------------------- */
        ConfigDocument runConfigDoc;
        auto runConfigElement = XmlTools::getElement(rootElement, "runconfig", &runConfigDoc);
        if (!runConfigElement) {
            throw BadConfigurationException("No runconfig in configuration file");
        }

        /* ------------------------------------------------------------------------
           Parsing base times section
           --------------------------------------------------------------------- */
        auto basetimesElement = XmlTools::getElement(runConfigElement, "basetimes");
        if (basetimesElement) {
            for (auto& basetimeElement : basetimesElement.children("basetime")) {
                _opaqRun.addBaseTime(chrono::from_date_time_string(basetimeElement.value()));
            }
        } else {
            _logger->warn("no base times section in configuration file"); // but might be given using command line args
        }

        /* ------------------------------------------------------------------------
           Parsing pollutant elements section
           --------------------------------------------------------------------- */
        auto name = std::string(runConfigElement.child("pollutant").value());
        if (name.empty()) {
            _logger->warn("no pollutant set in configuration file"); // but might be given using command line args
        } else {
            _opaqRun.setPollutantName(name); // set the pollutant name
        }

        name = std::string(runConfigElement.child("aggregation").value());
        if (name.empty()) {
            _logger->warn("no aggregation set in configuration file"); // but might be given using command line args
        } else {
            _opaqRun.setAggregation(name); // set the aggregation
        }

        /* ------------------------------------------------------------------------
           Parsing network section : selects the component which will deliver the
           AQ network configuration
           --------------------------------------------------------------------- */
        if (auto networkElement = runConfigElement.child("network"); networkElement) {
            _opaqRun.setNetworkProvider(_opaqRun.getComponent(networkElement.child("component").value()));
        } else {
            _logger->critical("No air quality network defined");
            throw RunTimeException("Invalid air quality network defined");
        }

        /* ------------------------------------------------------------------------
           Parsing grid section : selects the component which will deliver the
           Grid configuration
           --------------------------------------------------------------------- */
        if (auto gridElement = runConfigElement.child("grid"); gridElement) {
            _opaqRun.setGridProvider(_opaqRun.getComponent(gridElement.child("component").value()));
        } else {
            _logger->warn("No grid provider defined");
        }

        /* ------------------------------------------------------------------------
           Parsing forecast section
           --------------------------------------------------------------------- */
        if (auto forecastEl = runConfigElement.child("forecast"); forecastEl) {
            _opaqRun.setForecastStage(parseForecastStage(forecastEl));
        } else {
            _logger->warn("no forecast stage defined");
        }

        /* ------------------------------------------------------------------------
           Parsing mapping section
           --------------------------------------------------------------------- */
        if (auto mappingEl = runConfigElement.child("mapping"); mappingEl) {
            _opaqRun.setMappingStage(parseMappingStage(mappingEl));
        } else {
            _logger->warn("no mapping stage defined");
        }
    } catch (const std::exception& e) {
        throw BadConfigurationException("Unable to load configuration file: {}", e.what());
    }
}

void ConfigurationHandler::validateConfiguration(config::PollutantManager& pollutantMgr)
{
    // check for plugins with the same name
    auto plugins = _opaqRun.getPlugins();

    for (auto it1 = plugins.begin(); it1 != plugins.end(); ++it1) {
        auto name = it1->name;
        for (auto it2 = it1 + 1; it2 != plugins.end(); ++it2) {
            if (name == it2->name) {
                throw BadConfigurationException("Found 2 plugins with the same name: {}", name);
            }
        }

#ifndef STATIC_PLUGINS
        // check if plugin lib file exists
        if (!FileTools::exists(it1->libPath)) {
            throw BadConfigurationException("Library file not found: {}", it1->libPath);
        }
#endif
    }

    // check for components with the same name
    auto components = _opaqRun.getComponents();
    for (auto it1 = components.begin(); it1 != components.end(); ++it1) {
        auto name = it1->name;
        for (auto it2 = it1 + 1; it2 != components.end(); ++it2) {
            if (name == it2->name) {
                throw BadConfigurationException("Found 2 components with the same name: {}", name);
            }
        }
    }

    // check if base times are defined
    if (_opaqRun.getBaseTimes().empty()) {
        throw BadConfigurationException("No base times defined (need at least 1)");
    }

    // check if pollutant is defined
    if (!_opaqRun.pollutantIsSet()) {
        throw BadConfigurationException("No pollutant set");
    }

    // throws if not found
    pollutantMgr.find(_opaqRun.getPollutantName());
}
}
