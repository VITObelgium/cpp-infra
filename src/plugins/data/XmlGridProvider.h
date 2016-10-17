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

	// OPAQ::Component methods

    // throws BadConfigurationException
	void configure (TiXmlElement * configuration, const std::string& componentName, IEngine& engine) override;

	// OPAQ::GridProvider methods

	virtual Grid * getGrid() override {
		return &_grid;
	}

private:
	Grid _grid;
	Logger _logger;
};

} /* namespace OPAQ */
#endif /* XMLGRIDPROVIDER_H_ */
