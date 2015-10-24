/*
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#ifndef OPAQ_COMPONENT_H
#define OPAQ_COMPONENT_H

#include <string>
#include <tinyxml.h>
#include <vector>

#include "Plugin.h"

namespace OPAQ {


  /**
     The Config sub-namespace is a separate namespace to distinguish certain functions from the 
     main OPAQ namespace. 

     The main reason for it's existence is to make the distinction between the OPAQ::Component, 
     which is the actuall plugin component which provides the functionality and the 
     OPAQ::Config::Component which is in fact it's configuration which is read from the XML 
     config file.
   */
  namespace Config {


    /**
     * Component configuration class
     * Not to be confused with the OPAQ::Component class, which contains the acutall
     * component functionality. This is a configuration wrapper, which contains the 
     * XML configuration for the OPAQ Component. 
     */
    class Component {
    public:
      Component();
      virtual ~Component();
      
      /** Returns the XML configuration element for the Component configuration */
      TiXmlElement* getConfig() const { return config; }
      /** Set the components XML configuartion to the given tinyxml element */
      void setConfig(TiXmlElement* config) { this->config = config; }
      
      /** 
       * Retrieve the name for the component 
       * \note that the name is used in the OPAQ configuration file
       */
      std::string getName() const { return name; }
      /** Set the component name */
      void setName(std::string name) { this->name = name; }
      
      /**
       * Returns the plugin this component is based upon
       * \note What is returns is also the plugin configuartion element wrapper in the
       *       OPAQ::Config namespace
       */
      OPAQ::Config::Plugin* getPlugin() const { return plugin; }
      /** Set the Component's plugin to the given one */
      void setPlugin(OPAQ::Config::Plugin* plugin) { this->plugin = plugin; }
      
    private:
      std::string           name;
      OPAQ::Config::Plugin *plugin;
      TiXmlElement         *config;
    };
    
  } /* namespace Config */

} /* namespace OPAQ */
#endif /* OPAQ_COMPONENT_H */
