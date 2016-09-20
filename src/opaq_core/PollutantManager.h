#ifndef __POLLUTANTMANAGER_H
#define __POLLUTANTMANAGER_H

#include "Logger.h"
#include "Pollutant.h"

namespace OPAQ {

  namespace Config {
    /**
     * Singleton pollutant manager class
     * Presents the available pollutants, defined in the configuration file to the OPAQ framework
     */
    class PollutantManager {
    public:
      /**
	  Returns the instance of the singleton pollutant manager object.
	  As this is singleton class, there is no public constructor.
      */
      static PollutantManager *getInstance();

      virtual ~PollutantManager();

      /**
	  Output streamer for the pollutant manager
	  Streams a list of the pollutants to the os, e.g. called via
	  \param os output streamer
	  \param s  const reference to the pollutant manager
      */
      friend std::ostream& operator<<(std::ostream& os, const PollutantManager& s );

      /** Returns a reference to the list of the available pollutants
       */
      std::vector<OPAQ::Pollutant> &getList(){ return pollutants; }

      /** Searches for a pollutant of given name and returns a pointer to the pollutant object */
      OPAQ::Pollutant *find( std::string name );

      /** Configures the pollutant manager from the XML element
	  \param config const pointer to the TiXmlElement
	  This member function will push back OPAQ::Pollutants to the list for each
	  "<pollutant>" found in the "<pollutants>" section
      */
      void configure (TiXmlElement const * config);

    private:
      /** Private constructor for singleton class */
      PollutantManager();
      PollutantManager(PollutantManager const&); // no implementation of copy constructor for singleton
      void operator=(PollutantManager const&);   // no implementation of copy constructor for singleton

      // list of the available pollutants
      std::vector<OPAQ::Pollutant> pollutants; //!< list of available pollutants

      Logger logger;
    };

  } /* namespace Config */

} /* namespace OPAQ */

#endif /* #ifndef __POLLUTANTMANAGER_H */
