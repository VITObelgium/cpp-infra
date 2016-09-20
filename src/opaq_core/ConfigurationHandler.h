/*
 * ConfigurationHandler.h
 *
 *  Created on: Jan 15, 2014
 *      Author: vlooys
 */

#ifndef CONFIGURATIONHANDLER_H_
#define CONFIGURATIONHANDLER_H_

#include <tinyxml.h>
#include <string>
#include "tools/XmlTools.h"
#include "tools/FileTools.h"
#include "tools/DateTimeTools.h"
#include "config/OpaqRun.h"
#include "config/ForecastStage.h"
#include "config/MappingStage.h"
#include "PollutantManager.h"
#include "Logger.h"

namespace OPAQ {

  /**
   * Parses the master configuration file and constructs the main workflow objects in OPAQ
   * This class contains methods to parse the XML configuration file and subsequently construct
   * the main workflow objects : the plugins list, components, and the different runstages.
   */
  class ConfigurationHandler {
  public:
    ConfigurationHandler();
    virtual ~ConfigurationHandler() {
      std::vector<TiXmlDocument*>::iterator it = _configDocs.begin();
      while (it != _configDocs.end())
	delete *it++;
    }

    /**
     * Parse the configuration file given by the filename and construct the main OPAQ workflow
     * Here we indicate briefly the workflow of this method
     * - First we parse what is available to the OPAQ system in general,
     *   irrespective of the actual run requested. This defines what is available
     *   in the OPAQ configuration. This means
     *   - parsing the plugins section : "<plugins>"
     *   - parsing the components section : "<components>"
     *   - parsing the pollutants sections and populating the pollutant manager
     * -  Then we parse the run information, which defines how OPAQ should be run
     *    i.e. for what pollutant and what timesteps we should do ? Also this
     *    defines the forecast/mapping stages in the OPAQ run... Here we
     *    - parse the pollutant elements section, indicating what pollutant to run for
     *      (can be overwritten by command line)
     *    -  Parsing network section : selects the component which will deliver the
     *       AQ network configuration
     *    -  Parsing grid section : selects the component which will deliver the
     *       Grid configuration
     *    - parsing the forecast section : construt the forecast stage
     *    - and the mapping section : construc the mapping stage
     *
     * \param filename The name of the master XML configuration file
     */
    void parseConfigurationFile (std::string& filename);

    /** Validates whether the configuration is ok */
    void validateConfiguration ();

    /** Returns an opaq run object, constructed from the configuration parsing */
    OPAQ::Config::OpaqRun* getOpaqRun () { return &opaqRun; }

private:
    OPAQ::Config::OpaqRun opaqRun;
    TiXmlDocument doc;
    std::vector<TiXmlDocument *> _configDocs;
    Logger logger;

    void clearConfig();
    OPAQ::Config::Plugin*    findPlugin (std::string & pluginName);
    OPAQ::Config::Component* findComponent (std::string & componentName);

    Config::ForecastStage * parseForecastStage(TiXmlElement * element);
    Config::MappingStage  * parseMappingStage(TiXmlElement * element);
};


} /* namespace OPAQ */
#endif /* CONFIGURATIONHANDLER_H_ */
