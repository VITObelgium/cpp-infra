/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#ifndef OPAQ_SINGLETON_H
#define OPAQ_SINGLETON_H

#include <string>
#include <tinyxml.h>
#include <vector>

#include "Stage.h"
#include "Component.h"

namespace OPAQ {

  namespace Config {

    class Singleton : public Stage {
    public:
      Singleton();
      virtual ~Singleton();
      
      OPAQ::Config::Component* getModel() const { return model; }
      void setModel( OPAQ::Config::Component* model) { this->model = model; }
    
    private:
      OPAQ::Config::Component *model;
    };
    
  }

} /* namespace OPAQ */
#endif /* OPAQ_SINGLETON_H */
