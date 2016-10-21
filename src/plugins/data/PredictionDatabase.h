#pragma once

#include "Logger.h"
#include "DateTime.h"
#include "TimeSeries.h"

#include <vector>
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

    Logger _logger;
    sqlpp::sqlite3::connection _db;

    struct PreparedStatements;
    std::unique_ptr<PreparedStatements> _statements;
};

}
