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
    double lowerBound = 0.0;
    double upperBound = 0.0;
    inf::Color color;
    std::string name;
    std::string value; // if value is present lower and upper bound values are ignored

    bool operator==(const LegendEntry& other) const
    {
        return color == other.color &&
               name == other.name &&
               value == other.value &&
               std::abs(lowerBound - other.lowerBound) <= std::numeric_limits<double>::epsilon() &&
               std::abs(upperBound - other.upperBound) <= std::numeric_limits<double>::epsilon();
    }
};

struct Legend
{
    enum class Type
    {
        Categoric,  // Every legend entry represents a single value
        Numeric,    // Every legend entry represents a value range
        Contiguous, // Values are scaled contigously between a min and max value
    };

    /*! Map the value to a color based on the legend entries
     * If the value cannot be mapped the color of the edge entries
     * will be used depending on the the value
     * /param value the value to map
     */
    Color color_for_value(double value) const noexcept;
    Color color_for_value(std::string_view value) const noexcept;

    /*! Map the value to a color based on the legend entries
     * If the value cannot be mapped the unmappable color will be returned
     * /param value the value to map
     * /param unmappable the color that will be returned when the value is outside of the legend bands
     * /return the mapped color
     */
    Color color_for_value(double value, const Color& unmappable) const noexcept;
    Color color_for_value(std::string_view value, const Color& unmappable) const noexcept;

    /*! Map the value to a color based on the legend entries
     * If the value cannot be mapped the unmappable low or high color will be returned
     * /param value the value to map
     * /param unmappable the color that will be returned when the value is nan or outside of the legend bands
     * /param unmappableLow the color that will be returned when the value is outside of the legend bands and lower than the lowest legend band
     * /param unmappableHigh the color that will be returned when the value is outside of the legend bands and higher than the highest legend band
     * /return the mapped color
     */
    Color color_for_value(double value, const Color& unmappable, const Color& unmappableLow, const Color& unmappableHigh) const noexcept;

    Type type           = Type::Categoric;
    int numberOfClasses = 5;
    std::vector<LegendEntry> entries;
    ColorMap cmap;
    std::string colorMapName;
    std::string title;
    bool zeroIsNodata = false;

private:
    bool is_unmappable(double value) const noexcept;
};

Legend create_numeric_legend(double min, double max, int numberOfClasses, std::string_view cmapName, LegendScaleType method);
Legend create_numeric_legend(std::vector<float> sampleData, int numberOfClasses, std::string_view cmapName, LegendScaleType method);
Legend create_categoric_legend(int64_t min, int64_t max, std::string_view cmapName);
Legend create_legend(std::vector<float> sampleData, Legend::Type type, int numberOfClasses, std::string_view cmapName);
Legend create_contiguous_legend(std::string_view cmapName, double min, double max);

void generate_bounds(double min, double max, LegendScaleType method, Legend& legend);
void generate_bounds(std::vector<float> sampleData, LegendScaleType method, Legend& legend);
void generate_colors(std::string_view cmapName, Legend& legend);
void generate_legend_names(Legend& legend, int decimals, std::string_view unit);

}
