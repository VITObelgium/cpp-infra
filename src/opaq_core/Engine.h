/*
 * Engine.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#pragma once

#include <tinyxml.h>
#include <vector>

#include "config/ForecastStage.h"
#include "config/MappingStage.h"
#include "config/OpaqRun.h"

#include "data/DataProvider.h"
#include "data/ForecastOutputWriter.h"
#include "data/GridProvider.h"

#include "AQNetworkProvider.h"
#include "ComponentManager.h"
#include "Logger.h"
#include "Model.h"
#include "PollutantManager.h"

namespace OPAQ
{

class IEngine
{
public:
    virtual ~IEngine() = default;

    virtual Config::PollutantManager& pollutantManager() = 0;
    virtual ComponentManager&         componentManager() = 0;
};

/**
 * The main OPAQ abstract workflow class
 * The OPAQ engine contains the abstract implementation of the configurable
 * workflow for an
 * OPAQ run. It's run method is basically the body of an OPAQ run and the main
 * program is
 * just a wrapper around it.
 */
class Engine : public IEngine
{
public:
    Engine(Config::PollutantManager& pollutantMgr);
    
    /**
    * Prepare an opaq run
    * This method prepares the OPAQ workflow as configured in the configuration
    * object by
    * -# loading the plugins
    * -# instantiating and configuring the components
    * After you prepare a run, multiple runs can be executed
    */
    void prepareRun(Config::OpaqRun& config);

    /**
   * The main OPAQ run method given a configuration
   * This method executes the OPAQ workflow as configured in the configuration
   * object by
   * -# loading the plugins
   * -# instantiating and configuring the components
   * -# executing the main OPAQ workflow :
   *    - get the forecast/mapping stages
   *    - get the air quality network provider
   *    - get the grid provider
   *    - get the base times
   *    - get the requested basetimes the user wants to run and loop over them
   * executing the
   *      mapping/forecast stages...
   */

    void run(Config::OpaqRun& config);

    Config::PollutantManager& pollutantManager() override;
    ComponentManager&         componentManager() override;

private:
    Logger                    _logger;
    Config::PollutantManager& _pollutantMgr;
    ComponentManager          _componentMgr;

    /**
   * This runs the forecast stage with the given configuration, network,
   * pollutant and aggregation type for a specific basetime. The forecast stage
   * loops over the different models which are configured in the forecast config
   * and calls the forecast output writer.
   */
    void runForecastStage(Config::ForecastStage* cnf, AQNetworkProvider* net, Pollutant* pol, Aggregation::Type agg, DateTime& baseTime);

    void loadPlugins(const std::vector<Config::Plugin>& plugins);
    void initComponents(const std::vector<Config::Component>& components);
};

}
