#include "Engine.h"
#include "data/IGridProvider.h"
#include "data/IMappingBuffer.h"
#include "data/IStationInfoProvider.h"
#include "data/MeteoProvider.h"

#include "config/ForecastStage.h"
#include "config/MappingStage.h"
#include "config/OpaqRun.h"

#include "data/DataProvider.h"
#include "data/ForecastOutputWriter.h"
#include "data/IGridProvider.h"

#include "AQNetwork.h"
#include "AQNetworkProvider.h"
#include "Model.h"
#include "PollutantManager.h"
#include "infra/log.h"
#include "tools/ScopeGuard.h"

namespace opaq {

using namespace infra;
using namespace chrono_literals;

static const LogSource s_logSrc("Engine");

Engine::Engine(config::PollutantManager& pollutantMgr, const IPluginFactory& pluginFactory)
: _pollutantMgr(pollutantMgr)
, _compMgr(*this, pluginFactory)
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
    auto& obs = _compMgr.getComponent<DataProvider>(cnf.getValues().name);
    obs.setAQNetworkProvider(net);

    // Get meteo data provider (can be missing)
    MeteoProvider* meteo = nullptr;
    if (cnf.getMeteo()) {
        meteo = &_compMgr.getComponent<MeteoProvider>(cnf.getMeteo()->name);
        meteo->setBaseTime(baseTime);
    }

    // Get data buffer (can't be missing)
    auto& buffer = _compMgr.getComponent<ForecastBuffer>(cnf.getBuffer().name);
    buffer.setAQNetworkProvider(net);

    // Get the forecast models to run
    for (auto& modelConfig : cnf.getModels()) {
        auto& model = _compMgr.getComponent<Model>(modelConfig.name);

        // set ins and outs for the model
        model.setBaseTime(baseTime);
        model.setPollutant(pol);
        model.setAggregation(aggr);
        model.setAQNetworkProvider(net);
        model.setForecastHorizon(forecastHorizon);
        model.setInputProvider(obs);
        model.setMeteoProvider(meteo);
        model.setBuffer(&buffer);

        // Run the model up till the requested forecast horizon, the loop over the forecast horizons has to be
        // in the model as probably some models (AR) use info of previous days...
        Log::info(s_logSrc, "Running {}", model.getName());
        model.run();
    }

    // Prepare and run the forecast output writer for this basetime & pollutant
    auto& outWriter = _compMgr.getComponent<ForecastOutputWriter>(cnf.getOutputWriter().name);
    outWriter.setAQNetworkProvider(net);
    outWriter.setBuffer(&buffer);
    outWriter.setForecastHorizon(forecastHorizon);
    outWriter.write(pol, aggr, baseTime);
}

void Engine::runMappingStage(const config::MappingStage& cnf,
    AQNetworkProvider& aqNetworkProvider,
    IGridProvider& gridProvider,
    const Pollutant& pollutant,
    Aggregation::Type /*aggr*/,
    const chrono::date_time& baseTime)
{
    Log::info(s_logSrc, "Mapping");

    auto& buffer = _compMgr.getComponent<IMappingBuffer>(cnf.getMappingBuffer().name);
    auto& obs    = _compMgr.getComponent<DataProvider>(cnf.getDataProvider().name);
    obs.setAQNetworkProvider(aqNetworkProvider);

    for (auto& modelConfig : cnf.getModels()) {
        auto& model = _compMgr.getComponent<Model>(modelConfig.name);

        // set ins and outs for the model
        model.setBaseTime(baseTime);
        model.setPollutant(pollutant);
        model.setGridProvider(gridProvider);
        model.setAQNetworkProvider(aqNetworkProvider);
        model.setInputProvider(obs);
        model.setMappingBuffer(buffer);
        model.setGridType(cnf.getGridType());

        Log::info(s_logSrc, "Running {}", model.getName());
        model.run();
    }
}

void Engine::prepareRun(config::OpaqRun& config)
{
    initComponents(config.getComponents());
}

void Engine::run(config::OpaqRun& config)
{
    Log::info(s_logSrc, "Fetching workflow configuration");
    auto pollutant = _pollutantMgr.find(config.getPollutantName());

    auto forecastStage = config.getForecastStage();
    auto mappingStage  = config.getMappingStage();

    auto& aqNetworkProvider = _compMgr.getComponent<AQNetworkProvider>(config.getNetworkProvider()->name);
    Log::info(s_logSrc, "Using AQ network provider {}", aqNetworkProvider.getName());

    // Get grid provider
    IGridProvider* gridProvider = nullptr;
    auto gridProviderConfig     = config.getGridProvider();
    if (gridProviderConfig) {
        gridProvider = &_compMgr.getComponent<IGridProvider>(gridProviderConfig->name);
        Log::info(s_logSrc, "Using grid provider {}", gridProviderConfig->name);
    }

    // Get the base times
    auto baseTimes = config.getBaseTimes();

    IMappingBuffer* mappingBuffer = nullptr;
    if (mappingStage) {
        mappingBuffer = &_compMgr.getComponent<IMappingBuffer>(mappingStage->getMappingBuffer().name);
        auto& grid    = gridProvider->getGrid(pollutant.getName(), mappingStage->getGridType());
        mappingBuffer->openResultsFile(baseTimes.front(), baseTimes.back() + 1_d, pollutant, config.getAggregation(), aqNetworkProvider.getAQNetwork().getStations(), grid, mappingStage->getGridType());
    }

    auto closeBuffer = make_scope_guard([mappingBuffer]() {
        if (mappingBuffer) {
            mappingBuffer->closeResultsFile();
        }
    });

    Log::info(s_logSrc, "Starting OPAQ workflow...");
    if (forecastStage) {
        // Get data buffer
        auto& buffer = _compMgr.getComponent<ForecastBuffer>(forecastStage->getBuffer().name);

        for (auto& baseTime : baseTimes) {
            Log::info(s_logSrc, "Forecast stage for {}", chrono::to_date_string(baseTime));
            runForecastStage(*forecastStage, aqNetworkProvider, pollutant, config.getAggregation(), baseTime);

            if (mappingStage) {
                Log::info(s_logSrc, ">> Mapping forecast {}", chrono::to_date_string(baseTime));
                assert(gridProvider);

                // loop over the forecast horizons and call the mapping stage for each horizon
                for (auto fcHor = 0_d; fcHor <= forecastStage->getHorizon(); fcHor += 1_d) {
                    // Buffer is input provider for the mapping models
                    // set the forecast horizon on the hdf5 dataprovider, then run the mapping stage
                    buffer.setForecastHorizon(fcHor);
                    runMappingStage(*config.getMappingStage(), aqNetworkProvider, *gridProvider, pollutant, config.getAggregation(), baseTime);
                }
            }
        }
    } else if (mappingStage) {
        assert(gridProvider);

        for (auto& baseTime : baseTimes) {
            runMappingStage(*config.getMappingStage(), aqNetworkProvider, *gridProvider, pollutant, config.getAggregation(), baseTime);
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
    if (startTime > endTime) {
        throw RuntimeError("Validation start time must be before the end time");
    }

    auto days = chrono::to_days(endTime - startTime);

    std::vector<PredictionResult> results;
    results.reserve(days.count());

    auto valuesConfig = config.getForecastStage()->getValues();
    auto bufferConfig = config.getForecastStage()->getBuffer();

    auto& values = _compMgr.getComponent<DataProvider>(valuesConfig.name);
    auto& buffer = _compMgr.getComponent<ForecastBuffer>(bufferConfig.name);

    auto& aqNetworkProvider = _compMgr.getComponent<AQNetworkProvider>(config.getNetworkProvider()->name);
    values.setAQNetworkProvider(aqNetworkProvider);
    buffer.setAQNetworkProvider(aqNetworkProvider);
    buffer.setCurrentModel(model);

    auto measuredValues  = values.getValues(startTime, endTime, station, config.getPollutantName());
    auto predictedValues = buffer.getForecastValues(forecastHorizon, startTime, endTime, station, config.getPollutantName(), config.getAggregation());

    /*if (!measuredValues.isConsistent(predictedValues))
    {
        throw RuntimeError("Inconsistent measured and predicted values");
    }*/

    for (size_t i = 0; i < predictedValues.size(); ++i) {
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
    return _compMgr;
}

void Engine::initComponents(const std::vector<config::Component>& components)
{
    _compMgr.destroyComponents();
    for (auto& component : components) {
        try {
            Log::info(s_logSrc, "Creating component {} from plugin {}", component.name, component.plugin.name);
            _compMgr.createComponent<Component>(component.name, component.plugin.name, component.config);
        } catch (const std::exception& e) {
            Log::critical(s_logSrc, "Error while creating & configuring component {} ({})", component.name, e.what());
            throw std::runtime_error("Failed to initialize components");
        }
    }
}
}
