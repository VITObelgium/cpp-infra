/*
 * DataProvider.cpp
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#include "DataProvider.h"

namespace OPAQ {

DataProvider::DataProvider() {
	_AQNetworkProvider = 0;
	_currentModel      = "";
}

DataProvider::~DataProvider() {}

} /* namespace OPAQ */
