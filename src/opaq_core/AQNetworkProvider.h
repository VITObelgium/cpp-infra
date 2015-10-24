/*
 * Aqnetworkprovider.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#ifndef OPAQ_AQNETWORKPROVIDER_H_
#define OPAQ_AQNETWORKPROVIDER_H_

#include <Component.h>

#include "AQNetwork.h"

namespace OPAQ {

  // forward declaration
  class Component;

  /**
     The Air Quality Network provider
     Abstract base class for a network provider, a derived class should 
     implement the getAQNetwork routine which returns a pointer to an 
     Air Quality Network object. 

     A standard AQNetwork provider can e.g. contain a configure method which 
     simply reads the config section in the component configuration in the 
     main XML config file, however one could envisage having an air quality
     network provider which directly reads a sensor observation service and 
     gets it's station list from there
     
  */
  class AQNetworkProvider : virtual public OPAQ::Component {
  public:
    AQNetworkProvider ();
    virtual ~AQNetworkProvider() {}

    /** Pure virtual method which returns a pointer to an Air quality network object */
    virtual OPAQ::AQNetwork * getAQNetwork() = 0;

  };
  
} /* namespace OPAQ */
#endif /* OPAQ_AQNETWORKPROVIDER_H_ */
