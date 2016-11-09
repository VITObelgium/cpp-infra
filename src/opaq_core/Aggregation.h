#pragma once

#include "Exceptions.h"
#include <string>

namespace opaq
{

namespace Aggregation
{

enum Type
{
    None,
    DayAvg,
    Max1h,
    Max8h
};

/**
	 * Returns the preferred name of the aggregation type
	 */
std::string getName(Aggregation::Type agg);
std::string getDisplayName(Aggregation::Type agg);

/**
	 * Converts the string to the aggregation type, supports the old-school
	 * ones as well and is case in-sensitive. Supported are :
	 * - da, dayavg, dailyavg        -> OPAQ::AggregationType::DayAvg
	 * - m1, max1h, daymax, dailymax -> OPAQ::AggregationType::Max1h
	 * - m8, max8h                   -> OPAQ::AggregationType::Max8h
	 *
	 * \param s input string
	 */
Aggregation::Type fromString(std::string s);

}

}
