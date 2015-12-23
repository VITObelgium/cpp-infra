/*
 * Aggregation.h
 *
 *  Created on: Dec 21, 2015
 *      Author: bino
 */

#ifndef SRC_OPAQ_CORE_AGGREGATION_H_
#define SRC_OPAQ_CORE_AGGREGATION_H_

#include <string>

namespace OPAQ {

class Aggregation {
public:
	Aggregation();
	virtual ~Aggregation();

	enum Type { None, DailyAvg, Max1h, Max8h };

	static std::string getName( Aggregation::Type agg );
};

} /* namespace OPAQ */

#endif /* SRC_OPAQ_CORE_AGGREGATION_H_ */
