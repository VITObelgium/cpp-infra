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
			       DateTime              &baseTime ) {

  std::string name; 

  // Get the component manager
  ComponentManager *cm = ComponentManager::getInstance();
  
  // Get max forecast horizon
  ForecastHorizon forecastHorizon = cnf->getHorizon();

  // Get observation data provider
  name = cnf->getValues()->getName();
  DataProvider *obs = cm->getComponent<DataProvider>(name);
  obs->setAQNetworkProvider( net );
  obs->setBaseTime(baseTime);
  
  // Get meteo data provider (can be missing)
  DataProvider *meteo = NULL;
  try {
    Config::Component * component = cnf->getMeteo();
    std::string name = component->getName();
    meteo = cm->getComponent<DataProvider>(name);
    meteo->setAQNetworkProvider( net );
    meteo->setBaseTime(baseTime);
  } catch (NullPointerException & e) {}
  
  // Get databuffer (can't be missing)
  name = cnf->getBuffer()->getName();
  DataBuffer *buffer = cm->getComponent<DataBuffer>(name);
  buffer->setBaseTime(baseTime);
  buffer->setAQNetworkProvider( net );
  
  // Get the forecast models to run
  std::vector<Config::Component*>::iterator it =
    cnf->getModels().begin();

  // Vector to store which models we have run (will be passed on to outputwriter)
  std::vector<std::string> modelNames;

  while (it != cnf->getModels().end()) {
    // get mode
    name = (*it++)->getName();
    Model *model = cm->getComponent<Model>(name);
      
    // set ins and outs for the model
    model->setBaseTime(baseTime);
    model->setPollutant( *pol );
    model->setAQNetworkProvider( net );
    model->setForecastHorizon( &forecastHorizon );
    model->setInputProvider( obs );
    model->setMeteoProvider( meteo );
    model->setBuffer(buffer); 

    std::cout << " - running " << model->getName() << std::endl;

    // Run the model up till the requested forecast horizon...
    model->run();

    // List of models that were run, 
    // we get the component name back from the component
    modelNames.push_back( model->getName() );

  } // loop over the ensemble of models
  
  // Run ensemble mergers on the buffer
  // an ensemble merger should basically be a model as well, but one which is
  // aware of the other models and should give back exactly what to map
  


  // Prepare and run the forecast output writer for 
  // this basetime & pollutant
  name = cnf->getOutputWriter()->getName();
  ForecastOutputWriter *outWriter = cm->getComponent<ForecastOutputWriter>( name );
  outWriter->setAQNetworkProvider( net );
  outWriter->setBuffer( buffer );
  outWriter->setForecastHorizon( &forecastHorizon );
  outWriter->setModelNames( &modelNames ); // write output for these models

  std::cout << " - calling " << outWriter->getName() << " ..." << std::endl;
  outWriter->write( pol, baseTime );

  return;
}

/*
  void Engine::runStage(Config::Stage * stage, AQNetworkProvider * aqNetworkProvider,
		      GridProvider * gridProvider, DateTime & baseTime, Pollutant * pollutant,
		      ForecastHorizon * forecastHorizon ) {

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
    //name = ensemble->getMerger()->getName();
    //DataMerger * merger = cm->getComponent<DataMerger>(name);
    //merger->setBaseTime(baseTime);
    //merger->setGridProvider(gridProvider);
    
    
    
    // get ensemble output store (can't be missing)
    name = ensemble->getOutput()->getName();
    DataBuffer *buffer = cm->getComponent<DataStore>(name);
    buffer->setBaseTime(baseTime);
    buffer->setGridProvider(gridProvider);
    
    std::vector<Config::Component*>::iterator it =
      ensemble->getModels().begin();
    while (it != ensemble->getModels().end()) {
      // get mode
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

      //model->setOutputStore(merger);
      model->setOutputStore(buffer);
      
      // run the model up till the requested forecast horizon...
      model->run();


      // each model will dump it's data in the datastore
    }

    // run ensemble merger on the datastore
    
    

    // HERE we need an output writer...
    // optionally...
    // outputWriter->setPollutant( *pollutant );
    // outputWriter->setBaseTime( baseTime );
    // outputWriter->setDataBuffer( buffer )
    
    // the configuration of the output writer contains what to 
    // dump to the output...
    // outputWriter->write( *pollutant, model, baseTime )
      


    
    //if (fhCollector != NULL) {
    //  // collect forecast horizons during forecast stage
    //  fhCollector->setDataStore(output);
    //  merger->merge(fhCollector);
    //} else {
    //  // no need to do this during mapping stage
    //  merger->merge(output);
    //}
    

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
    DataBuffer * output = cm->getComponent<DataStore>(name);
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

    
    //if (fhCollector != NULL) {
    //  // collect forecast horizons during forecast stage
    //  fhCollector->setDataStore(output);
    //  model->setOutputStore(fhCollector);
    //} else {
    //  // no need to do this during mapping stage
    //  model->setOutputStore(output);
    //}
    
    
    // run the model
    model->run();
  }
  
}
 
*/


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
  OPAQ::ForecastHorizon fcHorMax = forecastStage->getHorizon();  

  logger->info("Starting OPAQ workflow...");
  std::cout << "Starting OPAQ workflow..." << std::endl;
  if (forecastStage) {
    
    for ( auto it = baseTimes.begin(); it != baseTimes.end(); it++ ) {
      DateTime baseTime = *it;

      // A log message
      logger->info( "Forecast stage for " + baseTime.dateToString() );
      std::cout << "Forecast stage for " << baseTime.dateToString() << std::endl;

      try {

	runForecastStage( forecastStage, aqNetworkProvider, pollutant, baseTime );

      } catch (std::exception & e) {

	logger->fatal("Unexpected error during forecast stage");
	logger->error(e.what());
	exit(1); 

      }
      
      if (mappingStage) {

	// a log message
	logger->info( "  Mapping stage for " + baseTime.dateToString() );
	std::cout << "Mapping stage for " << baseTime.dateToString() << std::endl;


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
      std::cout << "Mapping stage for " << baseTime.dateToString() << std::endl;
      
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
