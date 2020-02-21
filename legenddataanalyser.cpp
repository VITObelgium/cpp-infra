#include "infra/legenddataanalyser.h"

#include "infra/cast.h"
#include "infra/exception.h"

#include <algorithm>
#include <cmath>
#include <cassert>

namespace inf {

LegendDataAnalyser::LegendDataAnalyser(std::vector<float> sampleData)
: _sampleData(std::move(sampleData))
{
    // get and sort data
    if (!_sampleData.empty()) {
        std::sort(_sampleData.begin(), _sampleData.end());

        // check if frequency sorting is better
        size_t n                    = 1; // # distinct values
        const size_t sqrtOfDataSize = truncate<size_t>(std::floor(_sampleData.size() + 0.5));
        const size_t dataSize       = _sampleData.size();
        for (size_t i = 1; n < sqrtOfDataSize && i < dataSize; ++i) {
            if (_sampleData[i] != _sampleData[i - 1]) {
                ++n;
            }
        }

        if (n < sqrtOfDataSize) { // store data in frequency array
            _freqValue.resize(n);
            _freqValue[0] = _sampleData[0];
            _freqNr.resize(n);
            _freqNr[0] = 1;
            for (size_t i = 1, j = 0; i < dataSize; ++i) {
                if (_sampleData[i] != _sampleData[i - 1]) {
                    ++j;
                    _freqValue[j] = _sampleData[i];
                    _freqNr[j]    = 1;
                } else
                    ++_freqNr[j];
            }
        }
    }
}

void LegendDataAnalyser::set_number_of_classes(int n)
{
    _nClasses = std::max(0, n);
}

void LegendDataAnalyser::calculate_classbounds(LegendScaleType scaleType, double minValue, double maxValue)
{
    try {
        if (minValue >= maxValue) {
            minValue = _sampleData[0];
            maxValue = _sampleData[_sampleData.size() - 1];
        }
        _classBounds = inf::calculate_classbounds(scaleType, _nClasses, minValue, maxValue, _sampleData);
    } catch (const std::exception&) {
        _classBounds.clear();
    }
}

void LegendDataAnalyser::calculate_statistics(LegendScaleType scaleType, double minValue, double maxValue)
{
    if (_classBounds.empty()) {
        assert(false);
        _varByClass = 0;
        return;
    }

    if (_sampleData.size() < 1) {
        _varByClass = 0;
        return;
    }

    int n    = truncate<int>(_sampleData.size());
    int iMin = 0, iMax = n - 1;
    while (iMin < n && _sampleData[iMin] < minValue)
        ++iMin;
    while (iMax >= 0 && _sampleData[iMax] > maxValue)
        --iMax;
    n = iMax - iMin + 1;
    if (n < 1) {
        _varByClass = 0;
        return;
    }

#ifndef NDEBUG
    // check if classbounds are not decreasing
    for (int h = 0; h < _nClasses; h++)
        assert(_classBounds[h] <= _classBounds[h + 1]);
#endif

    std::vector<double> classAverage; // average of each class
    std::vector<int> classSize;       // size of the class
    classAverage.resize(_nClasses);
    classSize.resize(_nClasses);
    for (int i = 0; i < _nClasses; i++) {
        classAverage[i] = 0;
        classSize[i]    = 0;
    }
    double ave = 0; // overall average

    int j;

    // calculate sums
    j = 0;
    for (int i = iMin; i <= iMax; i++) {
        // find class for data element i
        while (_sampleData[i] > _classBounds[j + 1] && j < _nClasses - 1) { // if data is larger than max, put in largest class
            j++;
        }
        double temp = convert_value(scaleType, _sampleData[i], minValue);
        classAverage[j] += temp;
        classSize[j]++;
        ave += temp;
    }

    // calculate averages
    for (j = 0; j < _nClasses; j++) {
        if (classSize[j] > 0)
            classAverage[j] = classAverage[j] / classSize[j];
        else // no data
            classAverage[j] = (_classBounds[j] + _classBounds[j + 1]) / 2;
    }
    ave /= n;

    // calculate statistics
    _msw       = 0; // sum of squares within classes, mean after division
    _msb       = 0; // sum of squares between classes, mean after division
    double MST = 0; // sum of squares total, mean after division
    int i      = iMin;

    for (j = 0; j < _nClasses; j++) {
        _msb += classSize[j] * (classAverage[j] - ave) * (classAverage[j] - ave);
        for (int k = 0; k < classSize[j]; k++, i++) { // !!! i++
            double temp = convert_value(scaleType, _sampleData[i], minValue);
            _msw += (temp - classAverage[j]) * (temp - classAverage[j]);
            MST += (temp - ave) * (temp - ave);
        }
    }

    // rescale to get the mean
    if (_nClasses > 1) {
        _msb /= (_nClasses - 1);
    }
    if (n > _nClasses) {
        _msw /= (n - _nClasses);
    }
    if (n > 1) {
        MST /= (n - 1);
    }

    _varByClass = (MST > 0 && MST > _msw) ? 1 - (_msw / MST) : 0;
    assert(0 <= _varByClass && _varByClass <= 1);
}

bool LegendDataAnalyser::has_data() const
{
    return !_sampleData.empty();
}

float LegendDataAnalyser::min_value() const
{
    return _sampleData.front();
}

float LegendDataAnalyser::max_value() const
{
    return _sampleData.back();
}

double LegendDataAnalyser::convert_value(LegendScaleType scaleType, const double& value, const double& minValue) const
{
    if (scaleType == LegendScaleType::Linear) { // linear
        return value;
    } else if (scaleType == LegendScaleType::Arithmetic) { // arithmetic
        const double c = (minValue <= 0) ? 1 - minValue : 0;
        if (value > minValue + c) {
            return sqrt(value - minValue);
        }
    } else if (scaleType == LegendScaleType::Geometric || scaleType == LegendScaleType::Harmonic) { // geometric, harmonic: inverse does not exist
        const double c = (minValue < 1) ? 1 - minValue : 0;
        if (value + c > 1) {
            return log10(value + c);
        }
    } else if (scaleType == LegendScaleType::OverGeometric) { // over-geometric
        const double c = (minValue <= 0) ? 1 - minValue : 0;
        if (value > minValue) {
            return sqrt(log10(value + c) - log10(minValue + c));
        }
    } else {
        return value;
    }

    return 0.0; // consistency with user defined minValue (> value)
}

std::vector<std::tuple<double, double>> LegendDataAnalyser::classbounds() const
{
    std::vector<std::tuple<double, double>> bounds;
    bounds.reserve(_nClasses);

    if (!_classBounds.empty()) {
        assert(truncate<int>(_classBounds.size()) == _nClasses + 1);
        for (int i = 0; i < _nClasses; i++) {
            bounds.emplace_back(_classBounds[i], _classBounds[i + 1]);
        }
    }

    return bounds;
}

float LegendDataAnalyser::effectiveness() const
{
    return inf::truncate<float>(_varByClass);
}

static void assign_linear_class_bounds(int numClasses, double minValue, double maxValue, std::vector<double>& bounds)
{
    assert(bounds.size() == numClasses + 1);

    double x = (maxValue - minValue) / numClasses;
    for (int i = 1; i < numClasses; i++) {
        bounds[i] = minValue + i * x;
    }
}

static void assign_arithmetic_class_bounds(int numClasses, double minValue, double maxValue, std::vector<double>& bounds)
{
    double x = (maxValue - minValue) / (numClasses * (numClasses + 1.0) / 2.0);
    for (int i = 1; i < numClasses; i++) {
        bounds[i] = bounds[i - 1] + i * x;
    }
}

static void assign_linear_class_bounds_no_outliers(int numClasses, double minValue, double maxValue, const std::vector<float>& sampleData, std::vector<double>& bounds)
{
    // Perform a linear division between the configured low and high percentile

    constexpr double lowPercentile  = 0.05;
    constexpr double highPercentile = 0.95;

    assert(bounds.size() == numClasses + 1);
    size_t startIndex = truncate<size_t>(sampleData.size() * lowPercentile);
    size_t endIndex   = truncate<size_t>(sampleData.size() * highPercentile);

    minValue = sampleData[startIndex];
    maxValue = sampleData[endIndex];

    double x = (maxValue - minValue) / numClasses;
    for (int i = 1; i < numClasses; i++) {
        bounds[i] = minValue + i * x;
    }
}

std::vector<double> calculate_classbounds(LegendScaleType scaleType, int numClasses, double minValue, double maxValue)
{
    if (minValue >= maxValue) {
        throw InvalidArgument("Minimum class bound must be lower the maximum class bound ({} <-> {})", minValue, maxValue);
    }

    std::vector<double> classBounds(numClasses + 1);
    classBounds[0]          = minValue;
    classBounds[numClasses] = maxValue;

    if (scaleType == LegendScaleType::Linear) { // Linear sequence
        assign_linear_class_bounds(numClasses, minValue, maxValue, classBounds);
    } else if (scaleType == LegendScaleType::Arithmetic) { // Arithmetic sequence
        assign_arithmetic_class_bounds(numClasses, minValue, maxValue, classBounds);
    } else if (scaleType == LegendScaleType::Geometric) { // Geometric sequence
        if (minValue <= 0) {
            // calculate for minValue = 1 and then transpose back
            double logMax = log10(maxValue + 1 - minValue);
            double x      = pow(10., logMax / numClasses);
            for (int i = 1; i < numClasses; i++) {
                classBounds[i] = pow(x, (double)i);
            }
            for (int i = 1; i < numClasses; i++) {
                classBounds[i] -= 1 - minValue;
            }
        } else {
            double logMin = log10(minValue);
            double logMax = log10(maxValue);
            double x      = pow(10., (logMax - logMin) / numClasses);
            for (int i = 1; i < numClasses; i++) {
                classBounds[i] = classBounds[i - 1] * x;
            }
        }
    } else if (scaleType == LegendScaleType::OverGeometric) { // Over-Geometric sequence
        if (minValue <= 0) {
            // calculate for minValue = 1 and then transpose back
            double logMax  = log10(maxValue + 1 - minValue);
            double x       = pow(10., logMax / (numClasses * (numClasses + 1) / 2));
            classBounds[1] = x;
            for (int i = 2; i < numClasses; i++) {
                classBounds[i] = classBounds[i - 1] * pow(x, (double)i);
            }
            for (int i = 1; i < numClasses; i++) {
                classBounds[i] -= 1 - minValue;
            }
        } else {
            double logMin = log10(minValue);
            double logMax = log10(maxValue);
            double x      = pow(10., (logMax - logMin) / (numClasses * (numClasses + 1) / 2));
            for (int i = 1; i < numClasses; i++) {
                classBounds[i] = classBounds[i - 1] * pow(x, (double)i);
            }
        }
    } else if (scaleType == LegendScaleType::Harmonic) { // Harmonic sequence
        if (minValue <= 0) {
            // calculate for minValue = 1 and then transpose back
            double invMax = 1 / (maxValue + 1 - minValue);
            double x      = (1 - invMax) / numClasses;
            for (int i = 1; i < numClasses; i++) {
                classBounds[i] = 1 / (1 - i * x) - 1 + minValue;
            }
        } else {
            double invMin = 1 / minValue;
            double invMax = 1 / maxValue;
            double x      = (invMin - invMax) / numClasses;
            for (int i = 1; i < numClasses; i++) {
                classBounds[i] = 1 / (invMin - i * x);
            }
        }
    } else {
        throw InvalidArgument("Invalid legend scale type provided");
    }

    return classBounds;
}

std::vector<double> calculate_classbounds(LegendScaleType scaleType, int numClasses, double minValue, double maxValue, const std::vector<float>& sampleData)
{
    switch (scaleType) {
    case LegendScaleType::Quantiles:
    case LegendScaleType::StandardisedDescretisation:
    case LegendScaleType::MethodOfBertin:
    case LegendScaleType::LinearNoOutliers:
        // Only these methods require the sample data
        if (sampleData.empty()) {
            throw RuntimeError("No sample data provided");
        }
        break;
    default:
        // rely on the implementation without sample data
        return calculate_classbounds(scaleType, numClasses, minValue, maxValue);
    }

    std::vector<double> classBounds(numClasses + 1);
    classBounds[0]          = minValue;
    classBounds[numClasses] = maxValue;

    int n = truncate<int>(sampleData.size());

    if (scaleType == LegendScaleType::LinearNoOutliers) {
        assign_linear_class_bounds_no_outliers(numClasses, minValue, maxValue, sampleData, classBounds);
    } else if (scaleType == LegendScaleType::Quantiles) {
        int iMin = 0, iMax = n - 1;
        while (iMin < n && sampleData[iMin] < minValue) {
            ++iMin;
        }
        while (iMax >= 0 && sampleData[iMax] > maxValue) {
            --iMax;
        }
        n = iMax - iMin + 1;
        if (n < 1) {
            throw RuntimeError("Not enough sample data provided");
        }

        // put an equal amount of observations in each class
        float amplitude = float(n) / numClasses;
        float index     = float(iMin);
        for (int i = 1; i < numClasses; i++) {
            classBounds[i] = sampleData[int(index)]; //(sampleData[int(index)] + sampleData[int(index) + 1]) / 2;
            index += amplitude;
            if (index > iMax) {
                index = truncate<float>(iMax); // fix possible rounding error
            }
        }
    } else if (scaleType == LegendScaleType::StandardisedDescretisation) {
        int iMin = 0, iMax = n - 1;
        while (iMin < n && sampleData[iMin] < minValue)
            ++iMin;
        while (iMax >= 0 && sampleData[iMax] > maxValue)
            --iMax;
        n = iMax - iMin + 1;
        if (n < 1) {
            throw RuntimeError("Not enough sample data provided");
        }

        // calculate average and standard deviation
        double avg = 0;
        double sd  = 0;
        for (int i = iMin; i <= iMax; i++) {
            avg += sampleData[i];
            sd += sampleData[i] * sampleData[i];
        }
        avg = avg / n;
        sd  = sqrt(sd / n - avg * avg);

        for (int i = 1; i < numClasses; i++) {
            classBounds[i] = avg + (-numClasses / 2.0 + i) * sd;
            // Proof (with inductions).
            // Basis: with two classes classbound[1] = ave
            // Induction: each extra class results in:
            // classbound[1] getting a half sd earlier
            // classbound[numClasses-1] getting a half sd later
            // QED.
        }

        for (int h = 1; h < numClasses - 1; h++) {
            assert(fabs(classBounds[h] + sd - classBounds[h + 1]) < 0.0001);
        }
        // check if bounds are not lower than minValue or higher than maxValue
        for (int i = 1; i < numClasses; i++) {
            if (classBounds[i] < minValue)
                classBounds[i] = minValue;
            else if (classBounds[i] > maxValue)
                classBounds[i] = maxValue;
        }

        for (int h = 0; h < numClasses; h++) {
            assert(classBounds[h] <= classBounds[h + 1]);
        }
    } else if (scaleType == LegendScaleType::MethodOfBertin) {
        int iMin = 0, iMax = n - 1;
        while (iMin < n && sampleData[iMin] < minValue)
            ++iMin;
        while (iMax >= 0 && sampleData[iMax] > maxValue)
            --iMax;
        n = iMax - iMin + 1;
        if (n < 1) {
            throw RuntimeError("Not enough sample data provided");
        }

        // calculate average
        double avg = 0;
        for (int i = iMin; i <= iMax; ++i) {
            avg += sampleData[i];
        }
        avg /= n;

        // calculate class widths
        double xLeft  = 2 * (avg - minValue) / numClasses;
        double xRight = 2 * (maxValue - avg) / numClasses;

        double value = minValue;
        // left intervals
        int i;
        for (i = 1; i < (numClasses + 1) / 2.; i++) {
            value += xLeft;
            classBounds[i] = value;
        }
        if (numClasses % 2 == 1) {
            // center interval, i = (_nClasses + 1) / 2
            value += (xLeft + xRight) / 2;
            classBounds[i] = value;
            i++;
        }
        // right intervals
        for (; i < numClasses; i++) {
            value += xRight;
            classBounds[i] = value;
        }
    }

    return classBounds;
}
}
