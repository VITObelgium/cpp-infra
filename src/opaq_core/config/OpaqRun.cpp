#include "OpaqRun.h"
#include "../PollutantManager.h"

namespace OPAQ {

namespace Config {

OpaqRun::OpaqRun() :
   logger("OPAQ::Config::OpaqRun"),
   pollutantSet(false),
   aggregation(OPAQ::Aggregation::None),
   networkProvider(0),
   gridProvider(0),
   forecastStage(0),
   mappingStage(0) {
}

OpaqRun::~OpaqRun() {
	if (forecastStage) delete forecastStage;
	if (mappingStage) delete mappingStage;
}

}

} /* namespace OPAQ */


