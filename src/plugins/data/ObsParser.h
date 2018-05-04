#pragma once

#include "Aggregation.h"
#include "TimeSeries.h"

#include <cinttypes>
#include <fstream>
#include <string>
#include <unordered_map>

namespace opaq
{

class AQNetwork;

std::unordered_map<Aggregation::Type, std::unordered_map<std::string, TimeSeries<double>>> readObservationsFile(std::istream& file,
                                                                                                                const AQNetwork& aqNetwork,
                                                                                                                uint32_t numberOfValues,
                                                                                                                std::chrono::hours timeResolution);
}
