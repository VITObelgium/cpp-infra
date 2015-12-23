/*
 * Aggregation.cpp
 *
 *  Created on: Dec 21, 2015
 *      Author: bino
 */

#include "Aggregation.h"

namespace OPAQ {

Aggregation::Aggregation() {
}

Aggregation::~Aggregation() {
}

std::string Aggregation::getName( Aggregation::Type agg ) {

	switch( agg ) {
	case Type::None:
		return std::string( "None" );
		break;
	case Type::DailyAvg:
		return std::string( "DailyAvg" );
		break;
	case Type::Max1h:
		return std::string( "Max1h" );
		break;
	case Type::Max8h:
		return std::string( "Max8h" );
		break;
	}

	return std::string( "n/a" );
}

} /* namespace OPAQ */
