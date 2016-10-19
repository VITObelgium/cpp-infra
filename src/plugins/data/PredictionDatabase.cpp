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
using PreparedStatement = decltype(((sql::connection*)nullptr)->prepare(*(SelectType*)nullptr));

auto addPredictionQuery = []() {
    return insert_into(predictions).set(
        predictions.Basetime = parameter(predictions.Basetime),
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
        .where(predictions.Model == parameter(predictions.Model) and
               predictions.Pollutant == parameter(predictions.Pollutant) and
               predictions.Aggregation == parameter(predictions.Aggregation) and
               predictions.Station == parameter(predictions.Station) and
               predictions.ForecastHorizon == parameter(predictions.ForecastHorizon));
};

auto getPredictionValuesQuery = []() {
    return select(predictions.Value)
        .from(predictions)
        .where(predictions.Basetime == parameter(predictions.Basetime) and
               predictions.Pollutant == parameter(predictions.Pollutant) and
               predictions.Aggregation == parameter(predictions.Aggregation) and
               predictions.Station == parameter(predictions.Station) and
               predictions.ForecastHorizon == parameter(predictions.ForecastHorizon))
        .order_by(predictions.Model.asc());
};

auto getPredictionsInRangeQuery = []() {
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
    using GetPredictionValuesQuery = decltype(getPredictionValuesQuery());
    using GetPredictionsInRangeQuery = decltype(getPredictionsInRangeQuery());

    PreparedStatement<AddPredictionQuery> addPrediction;
    PreparedStatement<GetPredictionQuery> getPrediction;
    PreparedStatement<GetPredictionsQuery> getPredictions;
    PreparedStatement<GetPredictionValuesQuery> getPredictionValues;
    PreparedStatement<GetPredictionsInRangeQuery> getPredictionsInRange;
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
    _statements->getPredictionValues = _db.prepare(getPredictionValuesQuery());
    _statements->getPredictionsInRange = _db.prepare(getPredictionsInRangeQuery());
}

void PredictionDatabase::addPredictions(time_t baseTime,
                                        const std::string& model,
                                        const std::string& stationId,
                                        const std::string& pollutantId,
                                        const std::string& aggr,
                                        int fcHor,
                                        const TimeSeries<double>& forecast)
{
    _db.start_transaction();

    for (size_t i = 0; i < forecast.size(); ++i)
    {
        auto index = static_cast<unsigned int>(i);
    
        _statements->addPrediction.params.Basetime = baseTime;
        _statements->addPrediction.params.Date = forecast.datetime(index).getUnixTime();
        _statements->addPrediction.params.Value = forecast.value(index);
        _statements->addPrediction.params.Model = model;
        _statements->addPrediction.params.Pollutant = pollutantId;
        _statements->addPrediction.params.Aggregation = aggr;
        _statements->addPrediction.params.Station = stationId;
        _statements->addPrediction.params.ForecastHorizon = fcHor + index;

        _db(_statements->addPrediction);
    }

    _db.commit_transaction();
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

TimeSeries<double> PredictionDatabase::getPredictions(const std::string& model,
                                                      const std::string& stationId,
                                                      const std::string& pollutantId,
                                                      const std::string& aggr,
                                                      int fcHor)
{
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

std::vector<double> PredictionDatabase::getPredictionValues(time_t basetime,
                                                            const std::string& stationId,
                                                            const std::string& pollutantId,
                                                            const std::string& aggr,
                                                            int fcHor)
{
    _statements->getPredictionValues.params.Basetime = basetime;
    _statements->getPredictionValues.params.Pollutant = pollutantId;
    _statements->getPredictionValues.params.Aggregation = aggr;
    _statements->getPredictionValues.params.Station = stationId;
    _statements->getPredictionValues.params.ForecastHorizon = fcHor;

    std::vector<double> result;
    for (auto& row : _db(_statements->getPredictionValues))
    {
        result.push_back(row.Value);
    }

    return result;
}

TimeSeries<double> PredictionDatabase::getPredictions(time_t startDate,
                                                      time_t endDate,
                                                      const std::string& model,
                                                      const std::string& stationId,
                                                      const std::string& pollutantId,
                                                      const std::string& aggr,
                                                      int fcHor)
{
    _statements->getPredictionsInRange.params.startDate = startDate;
    _statements->getPredictionsInRange.params.endDate = endDate;
    _statements->getPredictionsInRange.params.Model = model;
    _statements->getPredictionsInRange.params.Pollutant = pollutantId;
    _statements->getPredictionsInRange.params.Aggregation = aggr;
    _statements->getPredictionsInRange.params.Station = stationId;
    _statements->getPredictionsInRange.params.ForecastHorizon = fcHor;

    TimeSeries<double> result;
    for (auto& row : _db(_statements->getPredictionsInRange))
    {
        result.insert(DateTime(row.Date), row.Value);
    }

    return result;
}

std::vector<std::string> PredictionDatabase::getModelNames(const std::string& pollutantId, const std::string& aggr)
{
    std::vector<std::string> names;

    auto result = _db(
        select(predictions.Model).flags(sqlpp::distinct)
        .from(predictions)
        .where(predictions.Aggregation == aggr and predictions.Pollutant == pollutantId)
        .order_by(predictions.Model.asc())
    );

    for (auto& row : result)
    {
        names.push_back(row.Model);
    }

    return names;
}

void PredictionDatabase::createInitialDatabase()
{
    _db.start_transaction();
    _db.execute("CREATE TABLE IF NOT EXISTS predictions("
                 "Basetime INTEGER,"
                 "Date INTEGER,"
                 "Value REAL,"
                 "Model TEXT,"
                 "Station TEXT,"
                 "Pollutant TEXT,"
                 "Aggregation TEXT,"
                 "ForecastHorizon INTEGER,"
                 "UNIQUE(Basetime, Date, Model, Station, Pollutant, Aggregation, ForecastHorizon) ON CONFLICT REPLACE);");

    _db.execute("CREATE INDEX IF NOT EXISTS BaseTimeIndex ON predictions(Basetime);");
    _db.execute("CREATE INDEX IF NOT EXISTS DateIndex ON predictions(Date);");
    _db.commit_transaction();
}

}
