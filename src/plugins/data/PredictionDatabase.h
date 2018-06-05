#pragma once

#include "DateTime.h"
#include "TimeSeries.h"

#include <sqlpp11/sqlite3/sqlite3.h>
#include <vector>

namespace sqlpp {
namespace sqlite3 {
class connection;
}
}

namespace opaq {

class PredictionDatabase
{
public:
    explicit PredictionDatabase(const std::string& filename);
    ~PredictionDatabase();

    void addPredictions(chrono::date_time baseTime,
        const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor,
        const TimeSeries<double>& forecast);

    double getPrediction(chrono::date_time date,
        const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor);

    TimeSeries<double> getPredictions(const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor);

    std::vector<double> getPredictionValues(chrono::date_time basetime,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor);

    TimeSeries<double> getPredictions(chrono::date_time startDate,
        chrono::date_time endDate,
        const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor);

    std::vector<std::string> getModelNames(const std::string& pollutantId, const std::string& aggr);

private:
    void createInitialDatabase();
    void prepareStatements();

    sqlpp::sqlite3::connection _db;

    struct PreparedStatements;
    std::unique_ptr<PreparedStatements> _statements;
};

}
