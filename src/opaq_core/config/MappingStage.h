/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#ifndef OPAQ_MAPPINGSTAGE_H
#define OPAQ_MAPPINGSTAGE_H

#include <string>
#include <tinyxml.h>
#include <vector>

#include "Component.h"
#include "../Exceptions.h"

namespace OPAQ {

namespace Config {

class MappingStage {
public:
  MappingStage();
  virtual ~MappingStage();

  // Throws OPAQ::NullPointerException
  OPAQ::Config::Component* getValues() const {
    if (values == NULL) throw OPAQ::NullPointerException();
    return values;
  }
  void setValues(OPAQ::Config::Component* values) {
    this->values = values;
  }

  // Throws OPAQ::NullPointerException
  OPAQ::Config::Component* getMeteo() const {
    if (meteo == NULL) throw OPAQ::NullPointerException();
    return meteo;
  }
  void setMeteo(OPAQ::Config::Component* meteo) {
    this->meteo = meteo;
  }
  
protected:


private:
  // input data provider components
  OPAQ::Config::Component *values;
  OPAQ::Config::Component *meteo;
  
};

} /* namespace Config */

} /* namespace OPAQ */
#endif /* OPAQ_STAGE_H */

