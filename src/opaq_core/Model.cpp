/*
 * Model.cpp
 *
 *  Created on: Jan 20, 2014
 *      Author: vlooys
 */

#include "Model.h"

namespace OPAQ {

  Model::Model() :
	  aggregation(Aggregation::None),
	  aqNetworkProvider(0),
	  gridProvider(0),
	  input(0),
	  meteo(0),
	  buffer(0),
	  missing_value(-9999) {
  }

  Model::~Model()
  {
  }

} /* namespace opaq */



