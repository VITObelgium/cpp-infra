#pragma once

#include "Pollutant.h"

namespace opaq {

namespace config {
/**
     * Singleton pollutant manager class
     * Presents the available pollutants, defined in the configuration file to the OPAQ framework
     */
class PollutantManager
{
public:
    PollutantManager()                        = default;
    PollutantManager(const PollutantManager&) = delete;
    void operator=(const PollutantManager&) = delete;

    /**
      Output streamer for the pollutant manager
      Streams a list of the pollutants to the os, e.g. called via
      \param os output streamer
      \param s  const reference to the pollutant manager
      */
    friend std::ostream& operator<<(std::ostream& os, const PollutantManager& s);

    /** Returns a reference to the list of the available pollutants
       */
    const std::vector<Pollutant>& getList()
    {
        return _pollutants;
    }

    /** Searches for a pollutant of given name and returns a pointer to the pollutant object */
    Pollutant find(const std::string& name);

    /** Configures the pollutant manager from the XML element
      \param config const pointer to the TiXmlElement
      This member function will push back OPAQ::Pollutants to the list for each
      "<pollutant>" found in the "<pollutants>" section
      */
    void configure(const infra::ConfigNode& config);

private:
    // list of the available pollutants
    std::vector<Pollutant> _pollutants; //!< list of available pollutants
};
}
}
