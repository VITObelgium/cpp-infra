#pragma once

#include <optional>

namespace opaq {

struct LegendSettings
{
    int categories;
    std::optional<double> minValue;
    std::optional<double> maxValue;
};

}
