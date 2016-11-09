/*
 * Aggregation.cpp
 *
 *  Created on: Dec 21, 2015
 *      Author: bino
 */

#include "Aggregation.h"
#include <algorithm>

namespace opaq
{
namespace Aggregation
{

std::string getName(Aggregation::Type agg)
{
    switch (agg)
    {
    case Type::None:    return std::string("none");
    case Type::DayAvg:  return std::string("dayavg");
    case Type::Max1h:   return std::string("max1h");
    case Type::Max8h:   return std::string("max8h");
    }

    return std::string("n/a");
}

std::string getDisplayName(Aggregation::Type agg)
{
    switch (agg)
    {
    case Type::None:    return std::string("None");
    case Type::DayAvg:  return std::string("Dayly average");
    case Type::Max1h:   return std::string("Maximum 1 hour");
    case Type::Max8h:   return std::string("Maximum 8 hours");
    }

    throw std::invalid_argument("Invalid aggregation type");
}

Aggregation::Type fromString(std::string s)
{
    if (s.empty())
    {
        return Aggregation::None;
    }

    // convert s to lower case
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);

    if (s == "da" || s == "dayavg" || s == "dailyavg") {
        return Aggregation::DayAvg;
    }
    else if (s == "m1" || s == "max1h" || s == "daymax" || s == "dailymax")
    {
        return Aggregation::Max1h;
    }
    else if (s == "m8" || s == "max8h")
    {
        return Aggregation::Max8h;
    }
    else if (s == "none")
    {
        return Aggregation::None;
    }

    throw NotAvailableException("Aggregation " + s + " is not known...");
}
}
}
