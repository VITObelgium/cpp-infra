/*
 * Engine.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#include "Engine.h"

namespace OPAQ
{

Engine::Engine(Config::PollutantManager& pollutantMgr)
: _logger("OPAQ::Engine")
, _pollutantMgr(pollutantMgr)
, _componentMgr(*this)
{
}

void Engine::runForecastStage(Config::ForecastStage* cnf,
                              AQNetworkProvider* net,
                              Pollutant* pol,
                              Aggregation::Type aggr,
                              DateTime& baseTime)
{
    // Get max forecast horizon...
    TimeInterval forecastHorizon = cnf->getHorizon();

    // Get observation data provider
    auto& obs = _componentMgr.getComponent<DataProvider>(cnf->getValues()->getName());
    obs.setAQNetworkProvider(net);

    // Get meteo data provider (can be missing)
    MeteoProvider* meteo = nullptr;
    try
    {
        Config::Component* component = cnf->getMeteo();
        std::string name             = component->getName();
        meteo                        = &_componentMgr.getComponent<MeteoProvider>(name);
        meteo->setBaseTime(baseTime);
    }
    catch (const NullPointerException&)
    {
    }

    // Get data buffer (can't be missing)
    auto& buffer = _componentMgr.getComponent<ForecastBuffer>(cnf->getBuffer()->getName());
    buffer.setAQNetworkProvider(net);

    // Get the forecast models to run
    for (auto* modelConfig : cnf->getModels())
    {
        auto& model = _componentMgr.getComponent<Model>(modelConfig->getName());

        // set ins and outs for the model
        model.setBaseTime(baseTime);
        model.setPollutant(*pol);
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
    auto& outWriter = _componentMgr.getComponent<ForecastOutputWriter>(cnf->getOutputWriter()->getName());
    outWriter.setAQNetworkProvider(net);
    outWriter.setBuffer(&buffer);
    outWriter.setForecastHorizon(forecastHorizon);
    outWriter.write(pol, aggr, baseTime);
}

/* =============================================================================
   MAIN WORKFLOW OF OPAQ
   ========================================================================== */
void Engine::run(Config::OpaqRun& config)
{
    // 1. Load plugins...
    auto& plugins = config.getPlugins();
    loadPlugins(plugins);

    // 2. Instantiate and configure components...
    auto& components = config.getComponents();
    initComponents(components);

    // 3. OPAQ workflow...
    _logger->info("Fetching workflow configuration");
    auto pollutantName = config.getPollutantName();

    auto* pollutant = _pollutantMgr.find(pollutantName);

    // Get stages
    Config::ForecastStage* forecastStage = config.getForecastStage();
    Config::MappingStage* mappingStage   = config.getMappingStage();

    // Get air quality network provider
    auto name                            = config.getNetworkProvider()->getName();
    AQNetworkProvider& aqNetworkProvider = _componentMgr.getComponent<AQNetworkProvider>(name);
    _logger->info("Using AQ network provider {}", aqNetworkProvider.getName());

    // Get grid provider
    GridProvider* gridProvider;
    Config::Component* gridProviderDef = config.getGridProvider();
    if (gridProviderDef != nullptr) {
        name         = config.getGridProvider()->getName();
        gridProvider = &_componentMgr.getComponent<GridProvider>(name);
        _logger->info("Using grid provider {}", name);
    }

    // Get the base times
    std::vector<DateTime> baseTimes = config.getBaseTimes();

    // Get the requested forecast horizon
    OPAQ::TimeInterval fcHorMax = forecastStage->getHorizon();

    _logger->info("Starting OPAQ workflow...");
    if (forecastStage)
    {
        for (auto& baseTime : baseTimes)
        {
            // A log message
            _logger->info("Forecast stage for " + baseTime.dateToString());
            runForecastStage(forecastStage, &aqNetworkProvider, pollutant, config.getAggregation(), baseTime);

            if (mappingStage)
            {

                // a log message
                _logger->info(">> Mapping forecast " + baseTime.dateToString());

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

        for (auto it = baseTimes.begin(); it != baseTimes.end(); it++)
        {
            DateTime baseTime = *it;

            // a log message
            _logger->info(">> Mapping " + baseTime.dateToString());

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

Config::PollutantManager& Engine::pollutantManager()
{
    return _pollutantMgr;
}

ComponentManager& Engine::componentManager()
{
    return _componentMgr;
}

void Engine::loadPlugins(const std::vector<Config::Plugin>& plugins)
{
    for (auto& plugin : plugins)
    {
        std::string name     = plugin.getName();
        std::string filename = plugin.getLib();
        try
        {
            _logger->info("Loading plugin {} from {}", name, filename);
            _componentMgr.loadPlugin(name, filename);
        }
        catch (std::exception& e)
        {
            _logger->critical("Error while loading plugin {}", name);
            _logger->error(e.what());
            exit(1);
        }
    }

    return;
}

void Engine::initComponents(const std::vector<Config::Component>& components)
{

    for (auto& component : components)
    {
        auto componentName = component.getName();
        auto pluginName    = component.getPlugin()->getName();
        auto config        = component.getConfig();

        try
        {
            _logger->info("Creating component {} from plugin {}", componentName, pluginName);
            _componentMgr.createComponent<Component>(componentName, pluginName, config);
        }
        catch (const std::exception& e)
        {
            _logger->critical("Error while creating & configuring component {}", componentName);
            _logger->error(e.what());
            exit(EXIT_FAILURE);
        }
    }
}

} /* namespace opaq */
