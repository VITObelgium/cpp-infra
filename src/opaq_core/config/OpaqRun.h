/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#ifndef OPAQ_OPAQRUN_H
#define OPAQ_OPAQRUN_H

#include <string>
#include <tinyxml.h>
#include <vector>

#include "Component.h"
#include "ForecastStage.h"
#include "MappingStage.h"
#include "Plugin.h"
#include "../Logger.h"
#include "../Pollutant.h"
#include "../Aggregation.h"
#include "../DateTime.h"

namespace OPAQ {

  namespace Config {

    /**
     * Class containing the main workflow for an OPAQ run
     * This class contains the different stages, the network providers, the grid provider
     * and all the information needed for executing an OPAQ run. The run method of the
     * OPAQ engine needs an object of this class to execute.
     */
    class OpaqRun {
    public:
      OpaqRun();
      virtual ~OpaqRun();
      
      LOGGER_DEC();

      /** Returns a list of available plugins */
      std::vector<OPAQ::Config::Plugin>    & getPlugins() { return plugins; }

      /** Returns a list of available components */
      std::vector<OPAQ::Config::Component> & getComponents() { return components; }

      /** Returns the pollutant name requested for this run */
      std::string getPollutantName() const { return pollutantName; }
      bool pollutantIsSet () { return pollutantSet; }

      /** Returns the aggreagation requested for this run */
      OPAQ::Aggregation::Type getAggregation() const { return aggregation; }

      /** Returns a list of basetimes */
      std::vector<OPAQ::DateTime> & getBaseTimes() { return baseTimes; }

      /** Returns the nework provider */
      OPAQ::Config::Component *getNetworkProvider() const { return networkProvider; }
      /** Returns the grid provider */
      OPAQ::Config::Component *getGridProvider() const { return gridProvider; }
    
      /** Retrieve the forecast configuration object */
      OPAQ::Config::ForecastStage* getForecastStage() const { return forecastStage;  }

      /** Retrieve the mapping stage object */
      OPAQ::Config::MappingStage* getMappingStage() const { return mappingStage; }

      /** Set the requested pollutant & aggregation for this run */
      void setPollutantName( const std::string& name, const std::string& aggr = "" ) {
    	  pollutantName = name;
    	  aggregation   = Aggregation::fromString( aggr );
    	  pollutantSet  = true;
      }

      /** logfilename */
      const std::string& getLogFile( ) const { return logFile; }
      void setLogFile( const std::string& fname ) { logFile = fname; }

      /**
       * Set aggregation separately...
       */
      void setAggregation( const std::string& aggr ) {
    	  aggregation = Aggregation::fromString( aggr );
      }

      /** 
       * Set the network provider from the component configuration 
       * \param component a pointer to a OPAQ::Config::Component object which is defined in the 
       *                  master XML configuration file
       */
      void setNetworkProvider( OPAQ::Config::Component *component ) { this->networkProvider = component; }
      /** Set the grid provider from the component configuration
       * \param component a pointer to a OPAQ::Config::Component object which is defined in the 
       *                  master XML configuration file
       */
      void setGridProvider (OPAQ::Config::Component * component) {this->gridProvider = component; }

      /** Set the forecast stage */
      void setForecastStage(OPAQ::Config::ForecastStage* forecastStage) { this->forecastStage = forecastStage;  }

      /** Set the mapping stage */
      void setMappingStage(OPAQ::Config::MappingStage* mappingStage) { this->mappingStage = mappingStage; }


private:
      std::vector<OPAQ::Config::Plugin>    plugins;    //!< list of available plugins
      std::vector<OPAQ::Config::Component> components; //!< list of available components

      std::string pollutantName;                  //!< requested name of the pollutant
      bool pollutantSet;                          //!< do we have the pollutant set in the run (also checks the aggreagtion)
      OPAQ::Aggregation::Type aggregation;        //!< the aggregation for the run

      std::vector<OPAQ::DateTime> baseTimes;      //!< list of basetimes to process in this run

      OPAQ::Config::Component *networkProvider;   //!< the network provider
      OPAQ::Config::Component *gridProvider;	  //!< the grid provider

      OPAQ::Config::ForecastStage *forecastStage; //!< this defines the forecast configuration in the OPAQ run
      OPAQ::Config::MappingStage  *mappingStage;  //!< this defines the mapping configuration in the OPAQ run

      std::string logFile;                        //!< name for the logfile...
    };


  } /* namespace Config */

} /* namespace OPAQ */

#endif /* OPAQ_OPAQRUN_H */
