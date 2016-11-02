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

#include "AQNetworkProvider.h"
#include "Logger.h"
#include "Model.h"
#include "PollutantManager.h"
#include "ComponentManagerFactory.h"

namespace opaq
{

class IEngine
{
public:
    virtual ~IEngine() = default;

    virtual config::PollutantManager& pollutantManager() = 0;
    virtual ComponentManager&         componentManager() = 0;
};

struct PredictionResult
{
    PredictionResult(const chrono::date_time& dt, double measured, double predicted)
    : time(dt)
    , measuredValue(measured)
    , predictedValue(predicted)
    {
    }

    double x() const noexcept { return measuredValue; }
    double y() const noexcept { return predictedValue; }

    chrono::date_time time;
    double measuredValue  = 0.0;
    double predictedValue = 0.0;
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
    Engine(config::PollutantManager& pollutantMgr);

    /**
    * Prepare an opaq run
    * This method prepares the OPAQ workflow as configured in the configuration
    * object by
    * -# loading the plugins
    * -# instantiating and configuring the components
    * After you prepare a run, multiple runs can be executed
    */
    void prepareRun(config::OpaqRun& config);

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

    void run(config::OpaqRun& config);

    /*
     * Validate the measured values agains the predicted values for the given station
     */
    std::vector<PredictionResult> validate(config::OpaqRun& config,
                                           chrono::days forecastHorizon,
                                           const std::string& station,
                                           chrono::date_time startTime,
                                           chrono::date_time endTime,
                                           const std::string& model);

    config::PollutantManager& pollutantManager() override;
    ComponentManager&         componentManager() override;

private:
    Logger                              _logger;
    config::PollutantManager&           _pollutantMgr;
    ComponentManager                    _componentMgr;

    /**
   * This runs the forecast stage with the given configuration, network,
   * pollutant and aggregation type for a specific basetime. The forecast stage
   * loops over the different models which are configured in the forecast config
   * and calls the forecast output writer.
   */
    void runForecastStage(config::ForecastStage* cnf, AQNetworkProvider& net, const Pollutant& pol, Aggregation::Type agg, const chrono::date_time& baseTime);

    void loadPlugins(const std::vector<config::Plugin>& plugins);
    void initComponents(const std::vector<config::Component>& components);
};

}
