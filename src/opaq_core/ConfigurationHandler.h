#pragma once

#include "config/OpaqRun.h"
#include "infra/xmldocument.h"

#include <string>

namespace opaq {

namespace config {
class PollutantManager;
}

/**
   * Parses the master configuration file and constructs the main workflow objects in OPAQ
   * This class contains methods to parse the XML configuration file and subsequently construct
   * the main workflow objects : the plugins list, components, and the different runstages.
   */
class ConfigurationHandler
{
public:
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
    void parseConfigurationFile(const std::string& filename, config::PollutantManager& pollutantMgr);

    /** Validates whether the configuration is ok */
    void validateConfiguration(config::PollutantManager& pollutantMgr);

    /** Returns an opaq run object, constructed from the configuration parsing */
    config::OpaqRun& getOpaqRun()
    {
        return _opaqRun;
    }

private:
    config::ForecastStage parseForecastStage(const inf::XmlNode& element);
    config::MappingStage parseMappingStage(const inf::XmlNode& element);

    config::OpaqRun _opaqRun;
    inf::XmlDocument _doc;
    std::vector<inf::XmlDocument> _configDocs;
};
}
