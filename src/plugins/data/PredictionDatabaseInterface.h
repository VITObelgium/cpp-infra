#pragma once

#include "DateTime.h"
#include "TimeSeries.h"
#include <vector>

namespace opaq {

class IPredictionDatabase
{
public:
    virtual ~IPredictionDatabase() = default;

    virtual void addPredictions(chrono::date_time baseTime,
        const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor,
        const TimeSeries<double>& forecast) = 0;

    virtual double getPrediction(chrono::date_time date,
        const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor) = 0;

    virtual TimeSeries<double> getPredictions(const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor) = 0;

    virtual std::vector<double> getPredictionValues(chrono::date_time basetime,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor) = 0;

    virtual TimeSeries<double> getPredictions(chrono::date_time startDate,
        chrono::date_time endDate,
        const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor) = 0;

    virtual std::vector<std::string> getModelNames(const std::string& pollutantId, const std::string& aggr) = 0;
};

}
