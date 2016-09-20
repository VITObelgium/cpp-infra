/**
 *  XMLAQNetProvider.h
 *
 *  Created on: Feb 19, 2014
 *      Author: maiheub
 *
 */

#ifndef __XMLAQNETPROVIDER_H
#define __XMLAQNETPROVIDER_H

#include <opaq.h>

// we can e.g. define namespaces per client for the plugin implementation...
namespace OPAQ {
  
  class XMLAQNetProvider : public OPAQ::AQNetworkProvider {
  public:
    XMLAQNetProvider();
    virtual ~XMLAQNetProvider();
    
    // component members
    // throws OPAQ::BadConfigurationException
    virtual void configure( TiXmlElement *configuration );
    
    // AQNetowrk functions
    virtual OPAQ::AQNetwork *getAQNetwork();
    
  private:
    OPAQ::AQNetwork _net;
    
    LOGGER_DEC();
  };
  
} /* namespace IRCEL */

LOGGER_DEF(OPAQ::XMLAQNetProvider);
#endif /* #ifndef __XMLAQNETPROVIDER_H */
