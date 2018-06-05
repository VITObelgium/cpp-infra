/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#pragma once

#include "../Aggregation.h"
#include "../DateTime.h"
#include "../Pollutant.h"
#include "Component.h"
#include "ForecastStage.h"
#include "MappingStage.h"
#include "Plugin.h"

#include <boost/optional.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace opaq {

namespace config {

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

    void addPlugin(const Plugin& plugin);
    void addComponent(const Component& component);

    Plugin getPlugin(const std::string& pluginName);
    const Component& getComponent(std::string_view componentName);

    std::vector<Plugin> getPlugins();
    std::vector<Component> getComponents();

    std::string getPollutantName() const;
    bool pollutantIsSet() const noexcept;

    Aggregation::Type getAggregation() const;

    std::vector<chrono::date_time> getBaseTimes() const;
    void addBaseTime(const chrono::date_time& dt);
    void clearBaseTimes();

    boost::optional<Component> getNetworkProvider() const;
    boost::optional<Component> getGridProvider() const;

    boost::optional<ForecastStage> getForecastStage() const;
    boost::optional<MappingStage> getMappingStage() const;

    void setPollutantName(const std::string& name, const std::string& aggr = "");
    void setAggregation(const std::string& aggr);

    void setNetworkProvider(const Component& component);
    void setGridProvider(const Component& component);

    void setForecastStage(ForecastStage forecastStage);
    void setMappingStage(MappingStage mappingStage);

private:
    std::vector<Plugin> _plugins;
    std::vector<Component> _components;

    std::string _pollutantName;
    Aggregation::Type _aggregation;

    std::vector<chrono::date_time> _baseTimes;

    boost::optional<Component> _networkProvider;
    boost::optional<Component> _gridProvider;

    boost::optional<ForecastStage> _forecastStage;
    boost::optional<MappingStage> _mappingStage;
};
}
}
