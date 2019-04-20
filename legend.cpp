#include "infra/legend.h"
#include "infra/cast.h"
#include "infra/exception.h"
#include "infra/legenddataanalyser.h"
#include "infra/log.h"
#include "infra/string.h"

#include <fmt/format.h>
#include <fstream>

namespace inf {

Legend create_numeric_legend(double min, double max, int numberOfClasses, std::string_view cmapName, LegendScaleType method)
{
    Legend legend;
    legend.type            = Legend::Type::Numeric;
    legend.numberOfClasses = numberOfClasses;
    legend.colorMapName    = cmapName;
    legend.entries.resize(numberOfClasses);
    legend.cmap = ColorMap::create(cmapName);

    generate_colors(cmapName, legend);
    generate_bounds(min, max, method, legend);

    return legend;
}

Legend create_numeric_legend(std::vector<float> sampleData, int numberOfClasses, std::string_view cmapName, LegendScaleType method)
{
    Legend legend;
    legend.type            = Legend::Type::Numeric;
    legend.numberOfClasses = numberOfClasses;
    legend.colorMapName    = cmapName;
    legend.entries.resize(numberOfClasses);
    legend.cmap = ColorMap::create(cmapName);

    generate_colors(cmapName, legend);
    generate_bounds(std::move(sampleData), method, legend);

    return legend;
}

Legend create_legend(std::vector<float> sampleData, Legend::Type type, int numberOfClasses, std::string_view cmapName)
{
    if (type == Legend::Type::Numeric) {
        return create_numeric_legend(std::move(sampleData), numberOfClasses, cmapName, LegendScaleType::Linear);
    }

    throw RuntimeError("Unsupported legend type");
}

Legend create_legend(std::vector<float> sampleData, int numberOfClasses, std::string_view cmapName, LegendScaleType method)
{
    Legend legend;
    legend.type            = Legend::Type::Numeric;
    legend.numberOfClasses = numberOfClasses;
    legend.colorMapName    = cmapName;
    legend.entries.resize(numberOfClasses);

    generate_colors(cmapName, legend);
    generate_bounds(std::move(sampleData), method, legend);

    return legend;
}

void generate_bounds(double min, double max, LegendScaleType method, Legend& legend)
{
    auto bounds = inf::calculate_classbounds(method, legend.numberOfClasses, min, max);
    assert(truncate<int>(bounds.size()) == legend.numberOfClasses + 1);
    assert(truncate<int>(legend.entries.size()) == legend.numberOfClasses);
    for (int i = 0; i < legend.numberOfClasses; ++i) {
        legend.entries[i].lowerBound = bounds[i];
        legend.entries[i].upperBound = bounds[i + 1];
    }
}

void generate_bounds(std::vector<float> sampleData, LegendScaleType scaleType, Legend& legend)
{
    LegendDataAnalyser dataAnalyser(std::move(sampleData));
    dataAnalyser.set_number_of_classes(legend.numberOfClasses);
    dataAnalyser.calculate_classbounds(scaleType);
    auto bounds = dataAnalyser.classbounds();

    size_t index = 0;
    for (auto& entry : legend.entries) {
        if (index < bounds.size()) {
            auto [lower, upper] = bounds[index++];
            entry.lowerBound    = lower;
            entry.upperBound    = upper;
        }
    }
}

void generate_colors(std::string_view cmapName, Legend& legend)
{
    auto cmap = inf::ColorMap::create(cmapName);

    const float colorOffset = legend.numberOfClasses == 1 ? 0.f : 1.f / (legend.numberOfClasses - 1.f);
    float colorPos          = 0.f;

    for (auto& entry : legend.entries) {
        auto color  = cmap.get_color(colorPos);
        entry.color = color;
        colorPos += colorOffset;
    }
}

void generate_legend_names(Legend& legend, int decimals, std::string_view unit)
{
    for (auto& entry : legend.entries) {
        entry.name = fmt::format("{:.{}f} ... {:.{}f} {}", entry.lowerBound, decimals, entry.upperBound, decimals, unit);
    }
}

}
