#include "infra/legend.h"
#include "infra/cast.h"
#include "infra/exception.h"
#include "infra/interpolate.h"
#include "infra/legenddataanalyser.h"
#include "infra/math.h"
#include "infra/string.h"

#include <cassert>
#include <fmt/format.h>
#include <fstream>

namespace inf {

Color Legend::color_for_value(double value) const noexcept
{
    if (is_unmappable(value) || entries.empty()) {
        return Color();
    }

    if (type == Type::Contiguous) {
        assert(entries.size() == 1);
        return cmap.get_color(linear_map_to_float(value, entries.front().lowerBound, entries.front().upperBound));
    }

    for (auto& entry : entries) {
        if (type == inf::Legend::Type::Categoric) {
            if (math::approx_equal(value, entry.lowerBound, std::numeric_limits<double>::epsilon())) {
                return entry.color;
            }
        } else if (type == inf::Legend::Type::Numeric) {
            if (value >= entry.lowerBound && value < entry.upperBound) {
                return entry.color;
            }
        }
    }

    if (type == inf::Legend::Type::Numeric) {
        if (value < entries.front().lowerBound) {
            return entries.front().color;
        }

        if (value >= entries.back().upperBound) {
            return entries.back().color;
        }
    }

    return Color();
}

Color Legend::color_for_value(std::string_view value) const noexcept
{
    if (value.empty()) {
        return Color();
    }

    if (type == inf::Legend::Type::Categoric) {
        for (auto& entry : entries) {
            if (entry.value == value) {
                return entry.color;
            }
        }
    }

    return Color();
}

Color Legend::color_for_value(double value, const Color& unmappable) const noexcept
{
    if (is_unmappable(value) || entries.empty()) {
        return unmappable;
    }

    if (type == Type::Contiguous) {
        assert(entries.size() == 1);
        return cmap.get_color(linear_map_to_float(value, entries.front().lowerBound, entries.front().upperBound));
    }

    for (auto& entry : entries) {
        if (type == inf::Legend::Type::Categoric) {
            if (math::approx_equal(value, entry.lowerBound, 1e-4)) {
                return entry.color;
            }
        } else if (type == inf::Legend::Type::Numeric) {
            if (value >= entry.lowerBound && value < entry.upperBound) {
                return entry.color;
            }
        }
    }

    if (value == entries.back().upperBound) {
        return entries.back().color;
    }

    return unmappable;
}

Color Legend::color_for_value(std::string_view value, const Color& unmappable) const noexcept
{
    if (value.empty()) {
        return unmappable;
    }

    for (auto& entry : entries) {
        if (type == inf::Legend::Type::Categoric) {
            if (entry.value == value) {
                return entry.color;
            }
        }
    }

    return unmappable;
}

Color Legend::color_for_value(double value, const Color& unmappable, const Color& unmappableLow, const Color& unmappableHigh) const noexcept
{
    if (is_unmappable(value) || entries.empty()) {
        return unmappable;
    }

    if (type == Type::Contiguous) {
        assert(entries.size() == 1);
        if (value >= (entries.front().lowerBound - 1e-4) && value <= (entries.front().upperBound + 1e-4)) {
            return cmap.get_color(linear_map_to_float(value, entries.front().lowerBound, entries.front().upperBound));
        }
    } else {
        for (auto& entry : entries) {
            if (type == inf::Legend::Type::Categoric) {
                if (math::approx_equal(value, entry.lowerBound, 1e-4)) {
                    return entry.color;
                }
            } else if (type == inf::Legend::Type::Numeric) {
                if (value >= entry.lowerBound && value < entry.upperBound) {
                    return entry.color;
                }
            }
        }
    }

    if (value < entries.front().lowerBound) {
        return unmappableLow;
    }

    if (value == entries.back().upperBound) {
        return entries.back().color;
    } else if (value >= entries.back().upperBound) {
        return unmappableHigh;
    }

    return unmappable;
}

bool Legend::is_unmappable(double value) const noexcept
{
    return std::isnan(value) || (zeroIsNodata && value == 0.0);
}

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
    legend.colorMapName    = cmapName;
    legend.numberOfClasses = numberOfClasses;
    legend.entries.resize(numberOfClasses);
    legend.cmap = ColorMap::create(cmapName);

    generate_colors(cmapName, legend);
    generate_bounds(std::move(sampleData), method, legend);

    return legend;
}

Legend create_categoric_legend(int64_t min, int64_t max, std::string_view cmapName)
{
    Legend legend;
    legend.type            = Legend::Type::Categoric;
    legend.numberOfClasses = truncate<int>((max - min) + 1);
    legend.colorMapName    = cmapName;
    legend.entries.resize(legend.numberOfClasses);
    legend.cmap = ColorMap::create(cmapName);

    const float colorOffset = legend.numberOfClasses == 1 ? 0.f : 1.f / (legend.numberOfClasses - 1.f);
    float colorPos          = 0.f;
    size_t entryIndex       = 0;
    for (int64_t i = min; i <= max; ++i) {
        legend.entries[entryIndex].color      = legend.cmap.get_color(colorPos);
        legend.entries[entryIndex].lowerBound = double(i);
        legend.entries[entryIndex].upperBound = legend.entries[entryIndex].lowerBound;
        colorPos += colorOffset;
        ++entryIndex;
    }

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

Legend create_contiguous_legend(std::string_view cmapName, double min, double max)
{
    Legend legend;

    LegendEntry entry;
    entry.lowerBound = min;
    entry.upperBound = max;

    legend.type = Legend::Type::Contiguous;
    legend.cmap = ColorMap::create(cmapName);
    legend.entries.push_back(entry);
    legend.colorMapName = cmapName;

    return legend;
}

void generate_bounds(double min, double max, LegendScaleType method, Legend& legend)
{
    if (min == max) {
        for (int i = 0; i < legend.numberOfClasses; ++i) {
            legend.entries[i].lowerBound = min;
            legend.entries[i].upperBound = min;
        }
    } else {
        auto bounds = inf::calculate_classbounds(method, legend.numberOfClasses, min, max);
        assert(truncate<int>(bounds.size()) == legend.numberOfClasses + 1);
        assert(truncate<int>(legend.entries.size()) == legend.numberOfClasses);
        for (int i = 0; i < legend.numberOfClasses; ++i) {
            legend.entries[i].lowerBound = bounds[i];
            legend.entries[i].upperBound = bounds[i + 1];
        }
    }
}

void generate_bounds(std::vector<float> sampleData, LegendScaleType scaleType, Legend& legend)
{
    LegendDataAnalyser dataAnalyser(std::move(sampleData));
    dataAnalyser.set_number_of_classes(legend.numberOfClasses);
    dataAnalyser.calculate_classbounds(scaleType);
    auto bounds            = dataAnalyser.classbounds();
    legend.numberOfClasses = truncate<int>(bounds.size());
    legend.entries.resize(legend.numberOfClasses);

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
    if (legend.type == Legend::Type::Numeric) {
        for (auto& entry : legend.entries) {
            entry.name = fmt::format("{:.{}f} ... {:.{}f} {}", entry.lowerBound, decimals, entry.upperBound, decimals, unit);
        }
    } else {
        for (auto& entry : legend.entries) {
            entry.name = fmt::format("{:.{}f} {}", entry.lowerBound, decimals, unit);
        }
    }
}

}
