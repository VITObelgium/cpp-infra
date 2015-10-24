#include "OpaqRun.h"
#include "../PollutantManager.h"

namespace OPAQ {

  namespace Config {

    OpaqRun::OpaqRun() {
    	pollutantSet = false;
    }
    
    OpaqRun::~OpaqRun() {
      if (forecastStage) delete forecastStage;
      if (mappingStage) delete mappingStage;
    }  

    LOGGER_DEF(OPAQ::Config::OpaqRun)

//    void OpaqRun::setPollutant( std::string name ) {
//      pollutant = OPAQ::Config::PollutantManager::getInstance()->find( name );
//      return;
//    }

  }
  
} /* namespace OPAQ */


