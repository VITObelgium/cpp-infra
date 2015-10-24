/*
 * Engine.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#include "Engine.h"

namespace OPAQ {

LOGGER_DEF(OPAQ::Engine)

void Engine::runStage(Config::Stage * stage, AQNetworkProvider * aqNetworkProvider,
		GridProvider * gridProvider, DateTime & baseTime, Pollutant * pollutant,
		ForecastHorizon * forecastHorizon, ForecastHorizonsCollector * fhCollector) {

	ComponentManager * cm = ComponentManager::getInstance();

	if (stage->isEnsemble()) {
		Config::Ensemble * ensemble = (Config::Ensemble *) stage;

		// get input values data provider (can't be missing)
		std::string name = ensemble->getValues()->getName();
		DataProvider * values = cm->getComponent<DataProvider>(name);
		values->setAQNetworkProvider(aqNetworkProvider);
		values->setBaseTime(baseTime);

		// get meteo data provider (can be missing)
		DataProvider * meteo = NULL;
		try {
			Config::Component * component = ensemble->getMeteo();
			std::string name = component->getName();
			meteo = cm->getComponent<DataProvider>(name);
			meteo->setAQNetworkProvider(aqNetworkProvider);
			meteo->setBaseTime(baseTime);
		} catch (NullPointerException & e) {}

		// get historical forecasts data provider (can be missing)
		DataProvider * historicalForecasts = NULL;
		try {
			Config::Component * component = ensemble->getHistoricalForecasts();
			std::string name = component->getName();
			historicalForecasts = cm->getComponent<DataProvider>(name);
			historicalForecasts->setAQNetworkProvider(aqNetworkProvider);
			historicalForecasts->setBaseTime(baseTime);
		} catch (NullPointerException & e) {}

		// get ensemble merger (can't be missing)
		name = ensemble->getMerger()->getName();
		DataMerger * merger = cm->getComponent<DataMerger>(name);
		merger->setBaseTime(baseTime);
		merger->setGridProvider(gridProvider);

		// get ensemble output store (can't be missing)
		name = ensemble->getOutput()->getName();
		DataStore * output = cm->getComponent<DataStore>(name);
		output->setBaseTime(baseTime);
		output->setGridProvider(gridProvider);

		std::vector<Config::Component*>::iterator it =
				ensemble->getModels().begin();
		while (it != ensemble->getModels().end()) {
			// get model
			name = (*it++)->getName();
			Model * model = cm->getComponent<Model>(name);

			// set ins and outs
			model->setBaseTime(baseTime);
			model->setPollutant(*pollutant);
			model->setAQNetworkProvider(aqNetworkProvider);
			model->setGridProvider(gridProvider);
			model->setForecastHorizon(forecastHorizon);
			model->setInputProvider(values);
			model->setMeteoProvider(meteo);
			model->setHistoricalForecastsProvider(historicalForecasts);
			model->setOutputStore(merger);

			// run the model
			model->run();
		}

		if (fhCollector != NULL) {
			// collect forecast horizons during forecast stage
			fhCollector->setDataStore(output);
			merger->merge(fhCollector);
		} else {
			// no need to do this during mapping stage
			merger->merge(output);
		}
	} else {
		// singleton model
		Config::Singleton * singleton = (Config::Singleton *) stage;

		// get input values data provider (can't be missing)
		std::string name = singleton->getValues()->getName();
		DataProvider * input = cm->getComponent<DataProvider>(name);
		input->setAQNetworkProvider(aqNetworkProvider);
		input->setBaseTime(baseTime);

		// get meteo data provider (can be missing)
		DataProvider * meteo = NULL;
		try {
			Config::Component * component = singleton->getMeteo();
			std::string name = component->getName();
			meteo = cm->getComponent<DataProvider>(name);
			meteo->setAQNetworkProvider(aqNetworkProvider);
			meteo->setBaseTime(baseTime);
		} catch (NullPointerException & e) {}

		// get historical forecasts data provider (can be missing)
		DataProvider * historicalForecasts = NULL;
		try {
			Config::Component * component = singleton->getHistoricalForecasts();
			std::string name = component->getName();
			historicalForecasts = cm->getComponent<DataProvider>(name);
			historicalForecasts->setAQNetworkProvider(aqNetworkProvider);
			historicalForecasts->setBaseTime(baseTime);
		} catch (NullPointerException & e) {}

		// get ensemble output store (can't be missing)
		name = singleton->getOutput()->getName();
		DataStore * output = cm->getComponent<DataStore>(name);
		output->setBaseTime(baseTime);
		output->setGridProvider(gridProvider);

		// get model
		name = singleton->getModel()->getName();
		Model * model = cm->getComponent<Model>(name);

		// set ins and outs
		model->setBaseTime(baseTime);
		model->setPollutant(*pollutant);
		model->setAQNetworkProvider(aqNetworkProvider);
		model->setGridProvider(gridProvider);
		model->setForecastHorizon(forecastHorizon);
		model->setInputProvider(input);
		model->setMeteoProvider(meteo);
		model->setHistoricalForecastsProvider(historicalForecasts);
		if (fhCollector != NULL) {
			// collect forecast horizons during forecast stage
			fhCollector->setDataStore(output);
			model->setOutputStore(fhCollector);
		} else {
			// no need to do this during mapping stage
			model->setOutputStore(output);
		}


		// run the model
		model->run();
	}

}

void Engine::run(Config::OpaqRun * config) {

	ComponentManager * cm = ComponentManager::getInstance();

	// 1. load plugins
	logger->info("loading plugins");
	std::vector<Config::Plugin> plugins = config->getPlugins();
	loadPlugins(&plugins);

	// 2. instantiate and configure components
	logger->info("creating & configuring components");
	std::vector<Config::Component> components = config->getComponents();
	initComponents(components);

	// 3. OPAQ workflow
	logger->info("fetching workflow configuration");
	std::string pollutantName = config->getPollutantName();

	Config::PollutantManager * pm = Config::PollutantManager::getInstance();
	Pollutant * pollutant = pm->find(pollutantName);
//	Pollutant * pollutant = config->getPollutant();

	// get stages
	Config::Stage * forecastStage = config->getForecastStage();
	Config::Stage * mappingStage = config->getMappingStage();

	// Get air quality network provider
	std::string name = config->getNetworkProvider()->getName();
	AQNetworkProvider * aqNetworkProvider = cm->getComponent<AQNetworkProvider>(
			name);

	// Get grid provider
	GridProvider * gridProvider = NULL;
	Config::Component * gridProviderDef = config->getGridProvider();
	if (gridProviderDef != NULL) {
		name = config->getGridProvider()->getName();
		gridProvider = cm->getComponent<GridProvider>(name);
	}

	// Get the base times
	std::vector<DateTime> baseTimes = config->getBaseTimes();

	// create forecast horizon collector
	ForecastHorizonsCollector fhCollector;

	logger->info("starting OPAQ workflow");
	if (forecastStage) {

		for (std::vector<DateTime>::iterator it = baseTimes.begin();
				it != baseTimes.end(); it++) {
			DateTime baseTime = *it;
			std::stringstream ss;
			ss << "base time = " << baseTime;
			logger->info(ss.str());
			std::vector<std::string> forecastedTimes;

			logger->info("running forecast stage");
			try {
				runStage(forecastStage, aqNetworkProvider, gridProvider, baseTime, pollutant, NULL, &fhCollector);
			} catch (std::exception & e) {
				logger->fatal("Unexpected error during forecast stage");
				logger->error(e.what());
				exit(1);
			}

			if (mappingStage) {
				logger->info("running mapping stage");
				const std::vector<ForecastHorizon> * fhs =
						&(fhCollector.getForecastHorizons());
				std::vector<ForecastHorizon>::const_iterator it = fhs->begin();
				while (it != fhs->end()) {
					ForecastHorizon fh = *it++;
					ss.str(std::string(""));
					ss << "forecast horizon = " << fh;
					logger->info(ss.str());
					try {
						runStage(mappingStage, aqNetworkProvider, gridProvider, baseTime, pollutant, &fh, NULL);
					} catch (std::exception & e) {
						logger->fatal("Unexpected error during mapping stage");
						logger->error(e.what());
						exit(1);
					}
				}
			}
		}

	} else {
		// no forecast stage defined
		if (mappingStage) {
			logger->info("running mapping stage");
			for (std::vector<DateTime>::iterator it = baseTimes.begin();
					it != baseTimes.end(); it++) {
				DateTime baseTime = *it;
				std::stringstream ss;
				ss << "base time = " << baseTime;
				logger->info(ss.str());
				/* EXECUTE MAPPING STAGE */
				ForecastHorizon fh (0);	// when mapping observations, the forecast horizon is always 0
				try {
					runStage(mappingStage, aqNetworkProvider, gridProvider, baseTime, pollutant, &fh, NULL);
				} catch (std::exception & e) {
					logger->fatal("Unexpected error during mapping stage");
					logger->error(e.what());
					exit(1);
				}

			}
		}
	}
}

void Engine::loadPlugins(std::vector<Config::Plugin> * plugins) {

	ComponentManager * cm = ComponentManager::getInstance();

	for (std::vector<Config::Plugin>::iterator it = plugins->begin();
			it != plugins->end(); it++) {
		Config::Plugin plugin = *it;

		std::string name = plugin.getName();
		std::string filename = plugin.getLib();
		try {
			cm->loadPlugin(name, filename);
		} catch (std::exception & e) {
			logger->fatal("Error while loading plugin " + name);
			logger->error(e.what());
			exit(1);
		}
	}

	return;
}

void Engine::initComponents(std::vector<Config::Component> & components) {

	ComponentManager * cm = ComponentManager::getInstance();

	for (std::vector<Config::Component>::iterator it = components.begin();
			it != components.end(); it++) {
		Config::Component component = *it;

		std::string componentName = component.getName();
		std::string pluginName = component.getPlugin()->getName();

		TiXmlElement * config = component.getConfig();

		try {
			cm->createComponent<Component>(componentName, pluginName, config);
		} catch (std::exception & e) {
			logger->fatal("Error while creating & configuring component " + componentName);
			logger->error(e.what());
			exit(1);
		}
	}

	return;
}

} /* namespace opaq */
