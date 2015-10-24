/*
 * GridProvider.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef GRIDPROVIDER_H_
#define GRIDPROVIDER_H_

#include "Component.h"
#include "../Grid.h"

namespace OPAQ {

class GridProvider: public OPAQ::Component {
public:
	GridProvider();
	virtual ~GridProvider();

	virtual Grid * getGrid() = 0;

};

} /* namespace OPAQ */
#endif /* GRIDPROVIDER_H_ */
