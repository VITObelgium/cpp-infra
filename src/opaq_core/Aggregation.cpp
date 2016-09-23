/*
 * Aggregation.cpp
 *
 *  Created on: Dec 21, 2015
 *      Author: bino
 */

#include <algorithm>
#include "Aggregation.h"

namespace OPAQ {
namespace Aggregation {

std::string getName( Aggregation::Type agg ) {

	switch( agg ) {
	case Type::None:
		return std::string( "none" );
		break;
	case Type::DayAvg:
		return std::string( "dayavg" );
		break;
	case Type::Max1h:
		return std::string( "max1h" );
		break;
	case Type::Max8h:
		return std::string( "max8h" );
		break;
	}

	return std::string( "n/a" );
}


Aggregation::Type fromString( std::string s ) {

	if ( s.size() == 0 ) return Aggregation::None;

	// convert s to lower case
	std::transform( s.begin(), s.end(), s.begin(), ::tolower );

	if ( !s.compare( "da" ) || !s.compare( "dayavg" ) || !s.compare( "dailyavg" ) ) {
		return Aggregation::DayAvg;
	} else if ( !s.compare( "m1" ) || !s.compare( "max1h" ) || !s.compare( "daymax" ) || !s.compare( "dailymax" ) ) {
		return Aggregation::Max1h;
	} else if ( !s.compare( "m8" ) || !s.compare( "max8h" ) ) {
		return Aggregation::Max8h;
	} else if ( !s.compare( "none" ) ) {
		return Aggregation::None;
	}

	throw NotAvailableException( "Aggregation " + s + " is not known..." );
}

}
} /* namespace OPAQ */
