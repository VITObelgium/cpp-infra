/*
 * Engine.cpp
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#include "Engine.h"

namespace OPAQ {

LOGGER_DEF(OPAQ::Engine)


void Engine::runForecastStage( Config::ForecastStage *cnf, 
							   AQNetworkProvider     *net,
							   Pollutant             *pol,
							   Aggregation::Type      aggr,
							   DateTime              &baseTime ) {

  std::string name; 

  // Get the component manager
  ComponentManager *cm = ComponentManager::getInstance();
  
  // Get max forecast horizon...
  TimeInterval forecastHorizon = cnf->getHorizon();

  // Get observation data provider
  name = cnf->getValues()->getName();
  DataProvider *obs = cm->getComponent<DataProvider>(name);
  obs->setAQNetworkProvider( net );
  
  // Get meteo data provider (can be missing)
  MeteoProvider *meteo = NULL;
  try {
    Config::Component * component = cnf->getMeteo();
    std::string name = component->getName();
    meteo = cm->getComponent<MeteoProvider>(name);
  } catch (NullPointerException & e) {}
  
  // Get data buffer (can't be missing)
  name = cnf->getBuffer()->getName();
  ForecastBuffer *buffer = cm->getComponent<ForecastBuffer>(name);
  buffer->setAQNetworkProvider( net );
  
  // Get the forecast models to run
  std::vector<Config::Component*>::iterator it = cnf->getModels().begin();

  while (it != cnf->getModels().end()) {
    // get mode
    name = (*it++)->getName();
    Model *model = cm->getComponent<Model>(name);
      
    // set ins and outs for the model
    model->setBaseTime(baseTime);
    model->setPollutant( *pol );
    model->setAggregation( aggr );
    model->setAQNetworkProvider( net );
    model->setForecastHorizon( forecastHorizon );
    model->setInputProvider( obs );
    model->setMeteoProvider( meteo );
    model->setBuffer(buffer); 

    // Run the model up till the requested forecast horizon, the loop over the forecast horizons has to be
    // in the model as probably some models (AR) use info of previous days...
    logger->info( " - running " + model->getName() );

    model->run();

  } // end of loop over the models...

  // Prepare and run the forecast output writer for 
  // this basetime & pollutant
  name = cnf->getOutputWriter()->getName();
  ForecastOutputWriter *outWriter = cm->getComponent<ForecastOutputWriter>( name );
  outWriter->setAQNetworkProvider( net );
  outWriter->setBuffer( buffer );
  outWriter->setForecastHorizon( forecastHorizon );
  std::cout << " - calling " << outWriter->getName() << " ..." << std::endl;
  outWriter->write( pol, aggr, baseTime );

  return;
}

/* =============================================================================
   MAIN WORKFLOW OF OPAQ
   ========================================================================== */
void Engine::run(Config::OpaqRun * config) {

  std::stringstream ss; // for logging

  ComponentManager *cm = ComponentManager::getInstance();

  // 1. Load plugins...
  logger->info("Loading plugins");
  std::cout << "Loading plugins..." << std::endl;
  std::vector<Config::Plugin> plugins = config->getPlugins();
  loadPlugins(&plugins);
  
  // 2. Instantiate and configure components...
  logger->info("Creating & configuring components");
  std::vector<Config::Component> components = config->getComponents();
  initComponents(components);
  
  // 3. OPAQ workflow...
  logger->info("Fetching workflow configuration");
  std::string pollutantName = config->getPollutantName();
  
  Config::PollutantManager * pm = Config::PollutantManager::getInstance();
  Pollutant * pollutant = pm->find(pollutantName);

  // Get stages
  Config::ForecastStage *forecastStage = config->getForecastStage();
  Config::MappingStage  *mappingStage  = config->getMappingStage();
  
  // Get air quality network provider
  std::string name = config->getNetworkProvider()->getName();
  AQNetworkProvider * aqNetworkProvider = cm->getComponent<AQNetworkProvider>(name);
  ss.str(""); ss.clear();
  ss << "Using AQ network provider " << aqNetworkProvider->getName();
  logger->info( ss.str() );


  // Get grid provider
  GridProvider *gridProvider;
  Config::Component * gridProviderDef = config->getGridProvider();
  if (gridProviderDef != NULL) {
    name = config->getGridProvider()->getName();
    gridProvider = cm->getComponent<GridProvider>(name);
    ss.str(""); ss.clear();
    ss << "Using grid provider " << gridProvider->getName();
    logger->info( ss.str() );
  }

  
  // Get the base times
  std::vector<DateTime> baseTimes = config->getBaseTimes();
  
  // Get the requested forecast horizon
  OPAQ::TimeInterval fcHorMax = forecastStage->getHorizon();

  logger->info("Starting OPAQ workflow...");
  std::cout << "Starting OPAQ workflow..." << std::endl;
  if (forecastStage) {
    
    for ( auto it = baseTimes.begin(); it != baseTimes.end(); it++ ) {
      DateTime baseTime = *it;

      // A log message
      logger->info( "Forecast stage for " + baseTime.dateToString() );
      try {

    	  runForecastStage( forecastStage, aqNetworkProvider, pollutant, config->getAggregation(), baseTime );

      } catch (std::exception & e) {

	logger->fatal("Unexpected error during forecast stage");
	logger->error(e.what());
	exit(1); 

      }
      
      if (mappingStage) {

	// a log message
	logger->info( "  Mapping stage for " + baseTime.dateToString() );

	// Buffer is input provider for the mapping models

	logger->fatal("No mapping stage implemented yet");
	exit(1);

	// we know what forecast horizons are requested by the user, no collector needed...

	/*
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
	*/
      }
    }
  } else {
    
    for ( auto it = baseTimes.begin(); it != baseTimes.end(); it++ ) {
      DateTime baseTime = *it;
      
      // a log message
      logger->info( "  Mapping stage for " + baseTime.dateToString() );
      
      logger->fatal("No mapping stage implemented yet");
      exit(1);

      /*
	ForecastHorizon fh (0);	// when mapping observations, the forecast horizon is always 0
	try {
	  runStage(mappingStage, aqNetworkProvider, gridProvider, baseTime, pollutant, &fh, NULL);
	} catch (std::exception & e) {
	  logger->fatal("Unexpected error during mapping stage");
	  logger->error(e.what());
	  exit(1);
	}
      */


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
