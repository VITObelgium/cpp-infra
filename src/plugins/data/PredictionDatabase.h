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

class DateTime;

class PredictionDatabase
{
public:
    explicit PredictionDatabase(const std::string& filename);
    ~PredictionDatabase();

    void addPredictions(time_t baseTime,
                        const std::string& model,
                        const std::string& stationId,
                        const std::string& pollutantId,
                        const std::string& aggr,
                        int fcHor,
                        const TimeSeries<double>& forecast);

    double getPrediction(time_t date,
                         const std::string& model,
                         const std::string& stationId,
                         const std::string& pollutantId,
                         const std::string& aggr,
                         int fcHor);

    TimeSeries<double> getPredictions(const std::string& model,
                                      const std::string& stationId,
                                      const std::string& pollutantId,
                                      const std::string& aggr,
                                      int fcHor);

    std::vector<double> getPredictionValues(time_t basetime,
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