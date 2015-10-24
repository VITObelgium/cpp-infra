/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: vlooys
 */

#ifndef OPAQ_ENSEMBLE_H
#define OPAQ_ENSEMBLE_H

#include <string>
#include <tinyxml.h>
#include <vector>

#include "Stage.h"
#include "Component.h"

namespace OPAQ {

  namespace Config {

    class Ensemble : public Stage {
    public:
      Ensemble();
      virtual ~Ensemble();
      
      OPAQ::Config::Component* getMerger() const { return merger; }
      void setMerger(OPAQ::Config::Component* merger) { this->merger = merger; }

      std::vector<OPAQ::Config::Component*> & getModels() { return models; }

    private:
      std::vector<OPAQ::Config::Component *> models;
      OPAQ::Config::Component * merger;
      
    };

  } /* namespace Config */

} /* namespace OPAQ */
#endif /* OPAQ_ENSEMBLE_H */
