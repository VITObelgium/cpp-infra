#include "infra/legend.h"
#include "infra/exception.h"
#include "infra/log.h"
#include "infra/string.h"
#include "legenddataanalyser.h"

#include <fmt/format.h>
#include <fstream>

namespace inf {

Legend create_numeric_legend(std::vector<float> sampleData, int numberOfClasses, std::string_view cmapName, LegendScaleType method)
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

void generate_bounds(std::vector<float> sampleData, LegendScaleType scaleType, Legend& legend)
{
    LegendDataAnalyser dataAnalyser(std::move(sampleData));
    dataAnalyser.set_number_of_classes(legend.numberOfClasses);
    dataAnalyser.calculate_classbounds(scaleType);
    auto bounds = dataAnalyser.classbounds();

    size_t index = 0;
    for (auto& entry : legend.entries) {
        auto [lower, upper] = bounds[index++];
        entry.lowerBound    = lower;
        entry.upperBound    = upper;
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
        entry.name = fmt::format("{:.{}f}{} ... {:.{}f}{}", entry.lowerBound, decimals, unit, entry.upperBound, decimals, unit);
    }
}

}