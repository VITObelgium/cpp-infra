#include "PredictionDatabase.h"

#include "PredictionScheme.h"

#include <sqlpp11/sqlpp11.h>

namespace OPAQ
{

namespace sql = sqlpp::sqlite3;

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

using AddPredictionQuery = decltype(addPredictionQuery());
using GetPredictionQuery = decltype(getPredictionQuery());

struct PredictionDatabase::PreparedStatements
{
    PreparedStatement<AddPredictionQuery> addPrediction;
    PreparedStatement<GetPredictionQuery> getPrediction;
};

PredictionDatabase::PredictionDatabase(const std::string& filename)
: _logger("PredictionDatabase")
, _db(createConfig(filename))
, _statements(std::make_unique<PreparedStatements>())
{
    prepareStatements();
    createInitialDatabase();
}

PredictionDatabase::~PredictionDatabase() = default;

void PredictionDatabase::prepareStatements()
{
    _statements->addPrediction = _db.prepare(addPredictionQuery());
    _statements->getPrediction = _db.prepare(getPredictionQuery());
}

void PredictionDatabase::addPrediction(time_t baseTime,
                                       time_t time,
                                       double value,
                                       const std::string& model,
                                       const std::string& stationId,
                                       const std::string& pollutantId,
                                       const std::string& aggr,
                                       int fcHor)
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

TimeSeries<double> PredictionDatabase::getPrediction(int fcHor,
                                                     time_t date,
                                                     const std::string& model,
                                                     const std::string& stationId,
                                                     const std::string& pollutantId,
                                                     const std::string& aggr)
{
    TimeSeries<double> result;

    _statements->getPrediction.params.Date = date;
    _statements->getPrediction.params.Model = model;
    _statements->getPrediction.params.Pollutant = pollutantId;
    _statements->getPrediction.params.Aggregation = aggr;
    _statements->getPrediction.params.Station = stationId;
    _statements->getPrediction.params.ForecastHorizon = fcHor;

    return result;
}

void PredictionDatabase::createInitialDatabase()
{
    _db.execute("CREATE TABLE IF NOT EXISTS predictions("
                 "Date INTEGER,"
                 "Value REAL,"
                 "Model TEXT,"
                 "Pollutant TEXT,"
                 "Aggregation TEXT,"
                 "ForecastHorizon INTEGER);");
}

}
