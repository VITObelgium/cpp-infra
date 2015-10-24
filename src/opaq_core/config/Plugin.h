/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#ifndef OPAQ_CONFIG_PLUGIN_H
#define OPAQ_CONFIG_PLUGIN_H

#include <string>
#include <tinyxml.h>
#include <vector>

namespace OPAQ {

  namespace Config {

    /**
     * Class to contains the plugin configuration
     * This class provides the interface with the plugin configuartion in the main OPAQ config file.
     */
    class Plugin {
    public:
      Plugin();
      virtual ~Plugin();
      
      /** Returns the library name
       *  being the path to the dll/so file which contains the functionality. This is set in the 
       *  main OPAQ configuration file. 
       */
      std::string getLib() const { return lib; }

      /** Set the library name to the given string */
      void setLib(std::string lib) { this->lib = lib; }
      
      /** Return the name for the plugin */
      std::string getName() const { return name; }

      /** Set the name for the plugin */
      void setName(std::string name) { this->name = name; }
      
    private:
      std::string name;
      std::string lib;
    };
  
  } /* namespace Config */

} /* namespace OPAQ */

#endif /* OPAQ_CONFIG_PLUGIN_H */
