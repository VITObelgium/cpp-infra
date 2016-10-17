#pragma once

#include "Logger.h"
#include "DateTime.h"
#include "TimeSeries.h"

#include <sqlpp11/sqlite3/sqlite3.h>

namespace sqlpp
{
namespace sqlite3
{
class connection;
}
}

namespace OPAQ
{

class DateTime;

class PredictionDatabase
{
public:
    explicit PredictionDatabase(const std::string& filename);
    ~PredictionDatabase();

    void addPrediction(time_t baseTime,
                       time_t time,
                       const std::string& model,
                       const std::string& stationId,
                       const std::string& pollutantId,
                       const std::string& aggr,
                       int fcHor,
                       double value);

    double getPrediction(time_t date,
                         const std::string& model,
                         const std::string& stationId,
                         const std::string& pollutantId,
                         const std::string& aggr,
                         int fcHor);

    TimeSeries<double> getPredictions(time_t startDate,
                                      time_t endDate,
                                      const std::string& model,
                                      const std::string& stationId,
                                      const std::string& pollutantId,
                                      const std::string& aggr,
                                      int fcHor);

private:
    void createInitialDatabase();
    void prepareStatements();

    Logger _logger;
    sqlpp::sqlite3::connection _db;

    struct PreparedStatements;
    std::unique_ptr<PreparedStatements> _statements;
};

}
