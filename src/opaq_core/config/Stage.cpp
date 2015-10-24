#include "Stage.h"

namespace OPAQ {

  namespace Config {

    Stage::Stage() {
    	values = NULL;
    	meteo = NULL;
    	historicalForecasts = NULL;
    	output = NULL;
    };
  
    Stage::~Stage() {}	// default implementation
    
  }

} /* namespace OPAQ */


