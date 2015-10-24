/*
 * XmlGridProvider.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef XMLGRIDPROVIDER_H_
#define XMLGRIDPROVIDER_H_

#include <opaq.h>
#include <algorithm>		// std::find

namespace OPAQ {

class XmlGridProvider: public OPAQ::GridProvider {
public:
	XmlGridProvider();
	virtual ~XmlGridProvider();

	LOGGER_DEC()

	// OPAQ::Component methods

	virtual void configure (TiXmlElement * configuration) throw (BadConfigurationException);

	// OPAQ::GridProvider methods

	virtual Grid * getGrid() {
		return &_grid;
	}

private:
	Grid _grid;

};

} /* namespace OPAQ */
#endif /* XMLGRIDPROVIDER_H_ */
