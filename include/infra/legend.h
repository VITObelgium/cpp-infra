#pragma once

#include "infra/color.h"
#include "infra/colormap.h"
#include "infra/legendscaletype.h"

#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace inf {

struct LegendEntry
{
    // in case of a categoric map, lower and upperbound are equal
    double lowerBound;
    double upperBound;
    inf::Color color;
    std::string name;

    bool operator==(const LegendEntry& other) const
    {
        return color == other.color &&
               name == other.name &&
               std::abs(lowerBound - other.lowerBound) <= std::numeric_limits<double>::epsilon() &&
               std::abs(upperBound - other.upperBound) <= std::numeric_limits<double>::epsilon();
    }
};

struct Legend
{
    enum class Type
    {
        Categoric,
        Numeric,
    };

    Type type           = Type::Categoric;
    int numberOfClasses = 5;
    std::vector<LegendEntry> entries;
    inf::ColorMap cmap;
    std::string colorMapName;
    bool zeroIsNodata = false;
};

Legend create_numeric_legend(double min, double max, int numberOfClasses, std::string_view cmapName, LegendScaleType method);
Legend create_numeric_legend(std::vector<float> sampleData, int numberOfClasses, std::string_view cmapName, LegendScaleType method);
Legend create_legend(std::vector<float> sampleData, Legend::Type type, int numberOfClasses, std::string_view cmapName);

void generate_bounds(double min, double max, LegendScaleType method, Legend& legend);
void generate_bounds(std::vector<float> sampleData, LegendScaleType method, Legend& legend);
void generate_colors(std::string_view cmapName, Legend& legend);
void generate_legend_names(Legend& legend, int decimals, std::string_view unit);

}
