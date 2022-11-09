#pragma once

#include "infra/legendscaletype.h"

#include <vector>

namespace inf {

class LegendDataAnalyser
{
public:
    LegendDataAnalyser(std::vector<float> sampleData);

    void set_number_of_classes(int n);

    void calculate_classbounds(LegendScaleType scaleType, double minValue = 0, double maxValue = -1);
    void calculate_statistics(LegendScaleType LegendScaleType, double minValue, double maxValue);

    bool has_data() const;
    float min_value() const;
    float max_value() const;

    std::vector<std::tuple<double, double>> classbounds() const;
    float effectiveness() const; // returns an estimate for the effectiveness of the class bounds

private:
    double convert_value(LegendScaleType scaleType, const double& value, const double& minValue) const;

    std::vector<float> _sampleData; // must be sorted small to large
    std::vector<int> _freqNr;       // # of occurrences of the values
    int _nClasses;
    std::vector<double> _classBounds; // size: _nClasses + 1

    // statistics
    double _msw;        // mean of squared deviations within classes
    double _msb;        // mean of squared deviations between classes
    double _varByClass; // proportion of variance accounted for by classification
};

std::vector<double> calculate_classbounds(LegendScaleType scaleType, int numClasses, double minValue, double maxValue);
std::vector<double> calculate_classbounds(LegendScaleType scaleType, int numClasses, double minValue, double maxValue, const std::vector<float>& sampleData);

}
