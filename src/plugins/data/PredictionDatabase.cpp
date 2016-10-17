#include "PredictionDatabase.h"

#include "PredictionScheme.h"

#include <sqlpp11/sqlpp11.h>

namespace OPAQ
{

//#define DEBUG_QUERIES

namespace sql = sqlpp::sqlite3;

SQLPP_ALIAS_PROVIDER(startDate);
SQLPP_ALIAS_PROVIDER(endDate);

static Predictions predictions;

static sql::connection_config createConfig(const std::string& filename)
{
    sql::connection_config config;
    config.path_to_database = filename;
    config.flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

#ifdef DEBUG_QUERIES
    config.debug = true;
#endif

    return config;
}

template <typename SelectType>
using PreparedStatement = decltype(((sql::connection*)nullptr)->prepare(*(reinterpret_cast<SelectType*>(nullptr))));

auto addPredictionQuery = []() {
    return insert_into(predictions).set(
        predictions.Date = parameter(predictions.Date),
        predictions.Value = parameter(predictions.Value),
        predictions.Model = parameter(predictions.Model),
        predictions.Pollutant = parameter(predictions.Pollutant),
        predictions.Aggregation = parameter(predictions.Aggregation),
        predictions.Station = parameter(predictions.Station),
        predictions.ForecastHorizon = parameter(predictions.ForecastHorizon)
    );
};

auto getPredictionQuery = []() {
    return select(predictions.Value)
        .from(predictions)
        .where(predictions.Date == parameter(predictions.Date) and
               predictions.Model == parameter(predictions.Model) and
               predictions.Pollutant == parameter(predictions.Pollutant) and
               predictions.Aggregation == parameter(predictions.Aggregation) and
               predictions.Station == parameter(predictions.Station) and
               predictions.ForecastHorizon == parameter(predictions.ForecastHorizon)
    );
};

auto getPredictionsQuery = []() {
    return select(predictions.Value, predictions.Date)
        .from(predictions)
        .where(predictions.Date >= parameter(sqlpp::integer(), startDate) and
               predictions.Date <= parameter(sqlpp::integer(), endDate) and
               predictions.Model == parameter(predictions.Model) and
               predictions.Pollutant == parameter(predictions.Pollutant) and
               predictions.Aggregation == parameter(predictions.Aggregation) and
               predictions.Station == parameter(predictions.Station) and
               predictions.ForecastHorizon == parameter(predictions.ForecastHorizon)
    );
};

struct PredictionDatabase::PreparedStatements
{
    using AddPredictionQuery = decltype(addPredictionQuery());
    using GetPredictionQuery = decltype(getPredictionQuery());
    using GetPredictionsQuery = decltype(getPredictionsQuery());

    PreparedStatement<AddPredictionQuery> addPrediction;
    PreparedStatement<GetPredictionQuery> getPrediction;
    PreparedStatement<GetPredictionsQuery> getPredictions;
};

PredictionDatabase::PredictionDatabase(const std::string& filename)
: _logger("PredictionDatabase")
, _db(createConfig(filename))
, _statements(std::make_unique<PreparedStatements>())
{
    createInitialDatabase();
    prepareStatements();
}

PredictionDatabase::~PredictionDatabase() = default;

void PredictionDatabase::prepareStatements()
{
    _statements->addPrediction = _db.prepare(addPredictionQuery());
    _statements->getPrediction = _db.prepare(getPredictionQuery());
    _statements->getPredictions = _db.prepare(getPredictionsQuery());
}

void PredictionDatabase::addPrediction(time_t baseTime,
                                       time_t time,
                                       const std::string& model,
                                       const std::string& stationId,
                                       const std::string& pollutantId,
                                       const std::string& aggr,
                                       int fcHor,
                                       double value)
{
    _statements->addPrediction.params.Date = time;
    _statements->addPrediction.params.Value = value;
    _statements->addPrediction.params.Model = model;
    _statements->addPrediction.params.Pollutant = pollutantId;
    _statements->addPrediction.params.Aggregation = aggr;
    _statements->addPrediction.params.Station = stationId;
    _statements->addPrediction.params.ForecastHorizon = fcHor;
    
    _db(_statements->addPrediction);
}

double PredictionDatabase::getPrediction(time_t date,
                                         const std::string& model,
                                         const std::string& stationId,
                                         const std::string& pollutantId,
                                         const std::string& aggr,
                                         int fcHor)
{
    _statements->getPrediction.params.Date = date;
    _statements->getPrediction.params.Model = model;
    _statements->getPrediction.params.Pollutant = pollutantId;
    _statements->getPrediction.params.Aggregation = aggr;
    _statements->getPrediction.params.Station = stationId;
    _statements->getPrediction.params.ForecastHorizon = fcHor;

    for (auto& row : _db(_statements->getPrediction))
    {
        // The result should be only one row
        return row.Value;
    }

    throw RunTimeException("Could not get prediction from database");
}

TimeSeries<double> PredictionDatabase::getPredictions(time_t startDate,
                                                      time_t endDate,
                                                      const std::string& model,
                                                      const std::string& stationId,
                                                      const std::string& pollutantId,
                                                      const std::string& aggr,
                                                      int fcHor)
{
    _statements->getPredictions.params.startDate = startDate;
    _statements->getPredictions.params.endDate = endDate;
    _statements->getPredictions.params.Model = model;
    _statements->getPredictions.params.Pollutant = pollutantId;
    _statements->getPredictions.params.Aggregation = aggr;
    _statements->getPredictions.params.Station = stationId;
    _statements->getPredictions.params.ForecastHorizon = fcHor;

    TimeSeries<double> result;
    for (auto& row : _db(_statements->getPredictions))
    {
        result.insert(DateTime(row.Date), row.Value);
    }

    return result;
}

void PredictionDatabase::createInitialDatabase()
{
    _db.execute("CREATE TABLE IF NOT EXISTS predictions("
                 "Date INTEGER,"
                 "Value REAL,"
                 "Model TEXT,"
                 "Station TEXT,"
                 "Pollutant TEXT,"
                 "Aggregation TEXT,"
                 "ForecastHorizon INTEGER);");
}

}
