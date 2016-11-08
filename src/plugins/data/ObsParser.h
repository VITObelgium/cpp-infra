#pragma once

#include "Aggregation.h"
#include "TimeSeries.h"

#include <map>
#include <fstream>
#include <string>
#include <cinttypes>

namespace opaq
{

class AQNetwork;

std::map<Aggregation::Type, std::map<std::string, TimeSeries<double>>> readObservationsFile(std::istream& file,
                                                                                            const AQNetwork& aqNetwork,
                                                                                            uint32_t numberOfValues,
                                                                                            std::chrono::hours timeResolution);

}