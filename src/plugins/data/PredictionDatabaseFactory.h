#pragma once

#include "DateTime.h"
#include "TimeSeries.h"
#include <vector>

namespace opaq {

class IPredictionDatabase;

namespace factory {
std::unique_ptr<IPredictionDatabase> createPredictionDatabase(std::string_view type, std::string_view location, std::string_view user, std::string_view pass);
}
}
