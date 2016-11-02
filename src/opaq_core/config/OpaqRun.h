/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#pragma once

#include "../Aggregation.h"
#include "../DateTime.h"
#include "../Logger.h"
#include "../Pollutant.h"
#include "Component.h"
#include "ForecastStage.h"
#include "MappingStage.h"
#include "Plugin.h"

#include <string>
#include <tinyxml.h>
#include <vector>

namespace opaq
{

namespace config
{

/**
  * Class containing the main workflow for an OPAQ run
  * This class contains the different stages, the network providers, the grid provider
  * and all the information needed for executing an OPAQ run. The run method of the
  * OPAQ engine needs an object of this class to execute.
  */
class OpaqRun
{
public:
    OpaqRun();

    void clear();

    void addPlugin(const Plugin& plugin);
    void addComponent(const Component& component);

    Plugin getPlugin(const std::string& pluginName);
    const Component& getComponent(const std::string& componentName);

    std::vector<Plugin> getPlugins();
    std::vector<Component> getComponents();

    /** Returns the pollutant name requested for this run */
    std::string getPollutantName() const { return _pollutantName; }
    bool pollutantIsSet() { return !_pollutantName.empty(); }

    /** Returns the aggreagation requested for this run */
    Aggregation::Type getAggregation() const { return _aggregation; }

    /** Returns a list of basetimes */
    const std::vector<chrono::date_time>& getBaseTimes() const { return _baseTimes; }
    void addBaseTime(const chrono::date_time& dt) { _baseTimes.push_back(dt); }
    void clearBaseTimes() { _baseTimes.clear(); }

    /** Returns the nework provider */
    Component* getNetworkProvider() const { return _networkProvider.get(); }
    /** Returns the grid provider */
    Component* getGridProvider() const { return _gridProvider.get(); }

    /** Retrieve the forecast configuration object */
    ForecastStage* getForecastStage() const { return _forecastStage.get(); }

    /** Retrieve the mapping stage object */
    MappingStage* getMappingStage() const { return _mappingStage.get(); }

    /** Set the requested pollutant & aggregation for this run */
    void setPollutantName(const std::string& name, const std::string& aggr = "")
    {
        _pollutantName = name;
        _aggregation   = Aggregation::fromString(aggr);
    }

    /**
       * Set aggregation separately...
       */
    void setAggregation(const std::string& aggr)
    {
        _aggregation = Aggregation::fromString(aggr);
    }

    /**
      * Set the network provider from the component configuration
      * \param component a Config::Component object which is defined in the master XML configuration file
      */
    void setNetworkProvider(const Component& component) { _networkProvider = std::make_unique<Component>(component); }
    void resetNetworkProvider() { _networkProvider.reset(); }
    
    /** Set the grid provider from the component configuration
      * \param component a Config::Component object which is defined in the master XML configuration file
      */
    void setGridProvider(const Component& component) { _gridProvider = std::make_unique<Component>(component); }
    void resetGridProvider() { _gridProvider.reset(); }


    /** Set the forecast stage */
    void setForecastStage(ForecastStage* forecastStage) { _forecastStage.reset(forecastStage); }

    /** Set the mapping stage */
    void setMappingStage(MappingStage* mappingStage) { _mappingStage.reset(mappingStage); }

private:
    Logger _logger;

    std::vector<Plugin> _plugins;
    std::vector<Component> _components;

    std::string _pollutantName;
    Aggregation::Type _aggregation;

    std::vector<chrono::date_time> _baseTimes;

    std::unique_ptr<Component> _networkProvider;
    std::unique_ptr<Component> _gridProvider;

    std::unique_ptr<ForecastStage> _forecastStage;
    std::unique_ptr<MappingStage> _mappingStage;
};

}
}
