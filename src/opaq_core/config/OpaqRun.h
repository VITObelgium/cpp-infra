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
#include "../DateTime.h"
#include "../ForecastHorizon.h"

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

      /** Set the requested pollutant name for this run */
      void setPollutantName( std::string & name ) {
    	  pollutantName = name;
    	  pollutantSet = true;
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

      std::string pollutantName;
      bool pollutantSet;

      std::vector<OPAQ::DateTime> baseTimes;

      OPAQ::Config::Component *networkProvider; //!< the network provider
      OPAQ::Config::Component *gridProvider;	//!< the grid provider

      OPAQ::Config::ForecastStage *forecastStage; //!< this defines the forecast configuration in the OPAQ run
      OPAQ::Config::MappingStage  *mappingStage;  //!< this defines the mapping configuration in the OPAQ run

    };


  } /* namespace Config */

} /* namespace OPAQ */

#endif /* OPAQ_OPAQRUN_H */
