#include "ForecastStage.h"

namespace OPAQ {

  namespace Config {
    
    ForecastStage::ForecastStage() {
      values = NULL;
      meteo  = NULL;
      buffer = NULL;
      outputWriter = NULL;
    };
    
    ForecastStage::~ForecastStage() {}	// default implementation
    
  }

} /* namespace OPAQ */


