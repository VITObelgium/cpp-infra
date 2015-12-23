/*
 * Engine.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#ifndef ENGINE_H_
#define ENGINE_H_

#include <vector>
#include <tinyxml.h>

#include <config/OpaqRun.h>
#include <config/ForecastStage.h>
#include <config/MappingStage.h>

#include <data/GridProvider.h>
#include <data/DataProvider.h>
#include <data/ForecastOutputWriter.h>

#include <Model.h>
#include <AQNetworkProvider.h>
#include <Logger.h>
#include <PollutantManager.h>
#include <ComponentManager.h>

namespace OPAQ {

  /**
   * The main OPAQ abstract workflow class
   * The OPAQ engine contains the abstract implementation of the configurable workflow for an 
   * OPAQ run. It's run method is basically the body of an OPAQ run and the main program is
   * just a wrapper around it. 
   */
  class Engine {
  public:
    Engine() {}
    virtual ~Engine() {}
    
    /**
     * The main OPAQ run method given a configuration
     * This method executes the OPAQ workflow as configured in the configuration object by
     * -# loading the plugins
     * -# instantiating and configuring the components
     * -# executing the main OPAQ workflow : 
     *    - get the forecast/mapping stages
     *    - get the air quality network provider
     *    - get the grid provider
     *    - get the base times
     *    - get the requested basetimes the user wants to run and loop over them executing the 
     *      mapping/forecast stages...
     */
    void run(Config::OpaqRun * config);
    
  private:
    LOGGER_DEC();
    
    /*
    void runStage(Config::Stage * stage, AQNetworkProvider * aqNetworkProvider,
		  GridProvider * gridProvider, DateTime & baseTime,
		  Pollutant * pollutant, ForecastHorizon * forecastHorizon,
		  ForecastHorizonsCollector * fhCollector);
    void runStage(Config::Stage * stage, AQNetworkProvider * aqNetworkProvider,
		  GridProvider * gridProvider, DateTime & baseTime,
		  Pollutant * pollutant, ForecastHorizon * forecastHorizon );

    */

    void runForecastStage( Config::ForecastStage *cnf, 
			   AQNetworkProvider     *net, 
			   Pollutant             *pol,
			   DateTime              &baseTime );
      

    void loadPlugins(std::vector<Config::Plugin> * plugins);
    void initComponents(std::vector<Config::Component> & components);
    
  };
  
} /* namespace OPAQ */
#endif /* ENGINE_H_ */
