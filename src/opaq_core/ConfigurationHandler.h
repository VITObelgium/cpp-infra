/*
 * ConfigurationHandler.h
 *
 *  Created on: Jan 15, 2014
 *      Author: vlooys
 */

#ifndef CONFIGURATIONHANDLER_H_
#define CONFIGURATIONHANDLER_H_

#include "Logger.h"
#include "PollutantManager.h"
#include "config/ForecastStage.h"
#include "config/MappingStage.h"
#include "config/OpaqRun.h"
#include "tools/DateTimeTools.h"
#include "tools/FileTools.h"
#include "tools/XmlTools.h"
#include <string>
#include <tinyxml.h>

namespace OPAQ
{

/**
   * Parses the master configuration file and constructs the main workflow objects in OPAQ
   * This class contains methods to parse the XML configuration file and subsequently construct
   * the main workflow objects : the plugins list, components, and the different runstages.
   */
class ConfigurationHandler
{
public:
    ConfigurationHandler();

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
    void parseConfigurationFile(const std::string& filename, Config::PollutantManager& pollutantMgr);

    /** Validates whether the configuration is ok */
    void validateConfiguration(Config::PollutantManager& pollutantMgr);

    /** Returns an opaq run object, constructed from the configuration parsing */
    Config::OpaqRun& getOpaqRun() { return _opaqRun; }

private:
    void clearConfig();

    Config::ForecastStage* parseForecastStage(TiXmlElement* element);
    Config::MappingStage* parseMappingStage(TiXmlElement* element);

    Config::OpaqRun _opaqRun;
    TiXmlDocument _doc;
    std::vector<std::unique_ptr<TiXmlDocument>> _configDocs;
    Logger _logger;
};

} /* namespace OPAQ */
#endif /* CONFIGURATIONHANDLER_H_ */
