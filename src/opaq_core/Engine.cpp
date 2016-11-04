/*
 * Engine.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#include "Engine.h"
#include "data/IGridProvider.h"
#include "ComponentManagerFactory.h"

namespace opaq
{

Engine::Engine(config::PollutantManager& pollutantMgr)
: _logger("OPAQ::Engine")
, _pollutantMgr(pollutantMgr)
, _componentMgr(factory::createComponentManager(*this))
{
}

void Engine::runForecastStage(const config::ForecastStage& cnf,
                              AQNetworkProvider& net,
                              const Pollutant& pol,
                              Aggregation::Type aggr,
                              const chrono::date_time& baseTime)
{
    // Get max forecast horizon...
    auto forecastHorizon = cnf.getHorizon();

    // Get observation data provider
    auto& obs = _componentMgr.getComponent<DataProvider>(cnf.getValues().name);
    obs.setAQNetworkProvider(net);

    // Get meteo data provider (can be missing)
    MeteoProvider* meteo = nullptr;
    try
    {
        meteo = &_componentMgr.getComponent<MeteoProvider>(cnf.getMeteo().name);
        meteo->setBaseTime(baseTime);
    }
    catch (const NullPointerException&)
    {
    }

    // Get data buffer (can't be missing)
    auto& buffer = _componentMgr.getComponent<ForecastBuffer>(cnf.getBuffer().name);
    buffer.setAQNetworkProvider(net);

    // Get the forecast models to run
    for (auto& modelConfig : cnf.getModels())
    {
        auto& model = _componentMgr.getComponent<Model>(modelConfig.name);

        // set ins and outs for the model
        model.setBaseTime(baseTime);
        model.setPollutant(pol);
        model.setAggregation(aggr);
        model.setAQNetworkProvider(net);
        model.setForecastHorizon(forecastHorizon);
        model.setInputProvider(&obs);
        model.setMeteoProvider(meteo);
        model.setBuffer(&buffer);

        // Run the model up till the requested forecast horizon, the loop over the forecast horizons has to be
        // in the model as probably some models (AR) use info of previous days...
        _logger->info("Running {}", model.getName());
        model.run();
    }

    // Prepare and run the forecast output writer for this basetime & pollutant
    auto& outWriter = _componentMgr.getComponent<ForecastOutputWriter>(cnf.getOutputWriter().name);
    outWriter.setAQNetworkProvider(net);
    outWriter.setBuffer(&buffer);
    outWriter.setForecastHorizon(forecastHorizon);
    outWriter.write(pol, aggr, baseTime);
}

/* =============================================================================
   MAIN WORKFLOW OF OPAQ
   ========================================================================== */

void Engine::prepareRun(config::OpaqRun& config)
{
    // 1. Load plugins...
    loadPlugins(config.getPlugins());

    // 2. Instantiate and configure components...
    initComponents(config.getComponents());
}

void Engine::run(config::OpaqRun& config)
{
    // OPAQ workflow...
    _logger->info("Fetching workflow configuration");
    auto pollutantName = config.getPollutantName();

    auto pollutant = _pollutantMgr.find(pollutantName);

    // Get stages
    auto forecastStage = config.getForecastStage();
    auto mappingStage  = config.getMappingStage();

    // Get air quality network provider
    auto name                            = config.getNetworkProvider()->name;
    AQNetworkProvider& aqNetworkProvider = _componentMgr.getComponent<AQNetworkProvider>(name);
    _logger->info("Using AQ network provider {}", aqNetworkProvider.getName());

    // Get grid provider
    IGridProvider* gridProvider;
    auto gridProviderDef = config.getGridProvider();
    if (gridProviderDef)
    {
        gridProvider = &_componentMgr.getComponent<IGridProvider>(gridProviderDef->name);
        _logger->info("Using grid provider {}", gridProviderDef->name);
    }

    // Get the base times
    auto baseTimes = config.getBaseTimes();

    _logger->info("Starting OPAQ workflow...");
    if (forecastStage)
    {
        for (auto& baseTime : baseTimes)
        {
            // A log message
            _logger->info("Forecast stage for {}", chrono::to_date_string(baseTime));
            runForecastStage(*forecastStage, aqNetworkProvider, pollutant, config.getAggregation(), baseTime);

            if (mappingStage)
            {

                // a log message
                _logger->info(">> Mapping forecast {}", chrono::to_date_string(baseTime));

                // Buffer is input provider for the mapping models

                _logger->critical("No mapping stage implemented yet");
                exit(1);

                // we know what forecast horizons are requested by the user, no collector needed...

                /*
  _logger->info("running mapping stage");
  const std::vector<ForecastHorizon> * fhs =
    &(fhCollector.getForecastHorizons());
  std::vector<ForecastHorizon>::const_iterator it = fhs->begin();
  while (it != fhs->end()) {
    ForecastHorizon fh = *it++;
    ss.str(std::string(""));
    ss << "forecast horizon = " << fh;
    _logger->info(ss.str());
    try {
      runStage(mappingStage, aqNetworkProvider, gridProvider, baseTime, pollutant, &fh, NULL);
    } catch (std::exception & e) {
      _logger->fatal("Unexpected error during mapping stage");
      _logger->error(e.what());
      exit(1);
    }
  }
  */
            }
        }
    }
    else
    {
        for (auto& bt : baseTimes)
        {
            _logger->info(">> Mapping {}", chrono::to_date_string(bt));
            _logger->critical("No mapping stage implemented yet");
            exit(1);

            /*
  ForecastHorizon fh (0); // when mapping observations, the forecast horizon is always 0
  try {
    runStage(mappingStage, aqNetworkProvider, gridProvider, baseTime, pollutant, &fh, NULL);
  } catch (std::exception & e) {
    _logger->fatal("Unexpected error during mapping stage");
    _logger->error(e.what());
    exit(1);
  }
      */
        }
    }
}

std::vector<PredictionResult> Engine::validate(config::OpaqRun& config,
                                               chrono::days forecastHorizon,
                                               const std::string& station,
                                               chrono::date_time startTime,
                                               chrono::date_time endTime,
                                               const std::string& model)
{
    if (startTime > endTime)
    {
        throw RunTimeException("Validation start time must be before the end time");
    }

    auto days = chrono::to_days(endTime - startTime);

    std::vector<PredictionResult> results;
    results.reserve(days.count());

    auto& valuesConfig = config.getForecastStage()->getValues();
    auto& bufferConfig = config.getForecastStage()->getBuffer();

    auto& values = _componentMgr.getComponent<DataProvider>(valuesConfig.name);
    auto& buffer = _componentMgr.getComponent<ForecastBuffer>(bufferConfig.name);

    auto& aqNetworkProvider = _componentMgr.getComponent<AQNetworkProvider>(config.getNetworkProvider()->name);
    values.setAQNetworkProvider(aqNetworkProvider);
    buffer.setAQNetworkProvider(aqNetworkProvider);
    buffer.setCurrentModel(model);

    auto measuredValues  = values.getValues(startTime, endTime, station, config.getPollutantName());
    auto predictedValues = buffer.getForecastValues(forecastHorizon, startTime, endTime, station, config.getPollutantName(), config.getAggregation());

    /*if (!measuredValues.isConsistent(predictedValues))
    {
        throw RunTimeException("Inconsistent measured and predicted values");
    }*/

    for (size_t i = 0; i < predictedValues.size(); ++i)
    {
        results.emplace_back(predictedValues.datetime(i), measuredValues.value(i), predictedValues.value(i));
    }

    return results;
}

config::PollutantManager& Engine::pollutantManager()
{
    return _pollutantMgr;
}

ComponentManager& Engine::componentManager()
{
    return _componentMgr;
}

void Engine::loadPlugins(const std::vector<config::Plugin>& plugins)
{
    for (auto& plugin : plugins)
    {
        try
        {
            _componentMgr.loadPlugin(plugin.name, plugin.libPath);
        }
        catch (std::exception& e)
        {
            throw RunTimeException("Failed to load plugins {} ({})", plugin.name, e.what());
        }
    }
}

void Engine::initComponents(const std::vector<config::Component>& components)
{
    _componentMgr.destroyComponents();
    for (auto& component : components)
    {
        try
        {
            _logger->info("Creating component {} from plugin {}", component.name, component.plugin.name);
            _componentMgr.createComponent<Component>(component.name, component.plugin.name, component.config);
        }
        catch (const std::exception& e)
        {
            _logger->critical("Error while creating & configuring component {} ({})", component.name, e.what());
            throw std::runtime_error("Failed to initialize components");
        }
    }
}

}
