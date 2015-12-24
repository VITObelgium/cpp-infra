/*
 * OVL.h
 *
 *  Created on: 2015
 *      Author: bino.maiheu@vito.be
 *
 *
 *  The OVL model, which makes use of the individual different FF models, but adds real time corrections &
 *  model selection
 */

#ifndef OVL_H_
#define OVL_H_

#include <string>
#include <opaq.h>

namespace OPAQ {
  
class OVL: virtual public OPAQ::Model {
public:
	OVL();
    virtual ~OVL();

    // OPAQ::Component methods
    virtual void configure (TiXmlElement * configuration)
      throw (OPAQ::BadConfigurationException);

    // the configure method should also be implemented in the derived class...
    // OPAQ::Model methods --> run for this particular fcTime...
    virtual void run();
    

private:
    LOGGER_DEC();
  };
  
  
} /* namespace OPAQ */
#endif /* OVL_H_ */
