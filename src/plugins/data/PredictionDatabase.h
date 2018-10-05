#pragma once

#include "DateTime.h"
#include "PredictionDatabaseInterface.h"
#include "PredictionScheme.h"
#include "TimeSeries.h"
#include "opaqconfig.h"

#ifdef OPAQ_ENABLE_POSTGRESQL
#include <sqlpp11/postgresql/postgresql.h>
#endif

#include <sqlpp11/sqlite3/sqlite3.h>
#include <sqlpp11/sqlpp11.h>

#include <vector>

namespace opaq {

SQLPP_ALIAS_PROVIDER(startDate);
SQLPP_ALIAS_PROVIDER(endDate);

static Predictions predictions;

static time_t to_epoch(const chrono::date_time& dt)
{
    return chrono::to_seconds(dt.time_since_epoch()).count();
}

static chrono::date_time from_epoch(time_t t)
{
    return chrono::date_time(std::chrono::seconds(t));
}

template <typename ConnectionType, typename SelectType>
using PreparedStatement = decltype(((ConnectionType*)nullptr)->prepare(*(SelectType*)nullptr));

static auto addPredictionQuery = []() {
    return insert_into(predictions).set(predictions.Basetime = parameter(predictions.Basetime), predictions.Date = parameter(predictions.Date), predictions.Value = parameter(predictions.Value), predictions.Model = parameter(predictions.Model), predictions.Pollutant = parameter(predictions.Pollutant), predictions.Aggregation = parameter(predictions.Aggregation), predictions.Station = parameter(predictions.Station), predictions.ForecastHorizon = parameter(predictions.ForecastHorizon));
};

static const auto getPredictionQuery = []() {
    return select(predictions.Value)
        .from(predictions)
        .where(predictions.Date == parameter(predictions.Date) and
               predictions.Model == parameter(predictions.Model) and
               predictions.Pollutant == parameter(predictions.Pollutant) and
               predictions.Aggregation == parameter(predictions.Aggregation) and
               predictions.Station == parameter(predictions.Station) and
               predictions.ForecastHorizon == parameter(predictions.ForecastHorizon));
};

static const auto getPredictionsQuery = []() {
    return select(predictions.Value, predictions.Date)
        .from(predictions)
        .where(predictions.Model == parameter(predictions.Model) and
               predictions.Pollutant == parameter(predictions.Pollutant) and
               predictions.Aggregation == parameter(predictions.Aggregation) and
               predictions.Station == parameter(predictions.Station) and
               predictions.ForecastHorizon == parameter(predictions.ForecastHorizon));
};

static const auto getPredictionValuesQuery = []() {
    return select(predictions.Value)
        .from(predictions)
        .where(predictions.Basetime == parameter(predictions.Basetime) and
               predictions.Pollutant == parameter(predictions.Pollutant) and
               predictions.Aggregation == parameter(predictions.Aggregation) and
               predictions.Station == parameter(predictions.Station) and
               predictions.ForecastHorizon == parameter(predictions.ForecastHorizon))
        .order_by(predictions.Model.asc());
};

static const auto getPredictionsInRangeQuery = []() {
    return select(predictions.Value, predictions.Date)
        .from(predictions)
        .where(predictions.Date >= parameter(sqlpp::integer(), startDate) and
               predictions.Date <= parameter(sqlpp::integer(), endDate) and
               predictions.Model == parameter(predictions.Model) and
               predictions.Pollutant == parameter(predictions.Pollutant) and
               predictions.Aggregation == parameter(predictions.Aggregation) and
               predictions.Station == parameter(predictions.Station) and
               predictions.ForecastHorizon == parameter(predictions.ForecastHorizon));
};

template <typename ConnectionType>
class PredictionDatabase : public IPredictionDatabase
{
public:
    template <typename ConfigType>
    explicit PredictionDatabase(ConfigType config)
    : _db(config)
    {
        createInitialDatabase();
        prepareStatements();
    }

    void addPredictions(chrono::date_time baseTime,
        const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor,
        const TimeSeries<double>& forecast) override
    {
        auto transaction = start_transaction(_db);

        for (size_t i = 0; i < forecast.size(); ++i) {
            auto index = static_cast<unsigned int>(i);

            _statements.addPrediction.params.Basetime        = to_epoch(baseTime);
            _statements.addPrediction.params.Date            = to_epoch(forecast.datetime(index));
            _statements.addPrediction.params.Value           = forecast.value(index);
            _statements.addPrediction.params.Model           = model;
            _statements.addPrediction.params.Pollutant       = pollutantId;
            _statements.addPrediction.params.Aggregation     = aggr;
            _statements.addPrediction.params.Station         = stationId;
            _statements.addPrediction.params.ForecastHorizon = fcHor.count() + index;

            _db(_statements.addPrediction);
        }

        transaction.commit();
    }

    double getPrediction(chrono::date_time date,
        const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor) override
    {
        _statements.getPrediction.params.Date            = to_epoch(date);
        _statements.getPrediction.params.Model           = model;
        _statements.getPrediction.params.Pollutant       = pollutantId;
        _statements.getPrediction.params.Aggregation     = aggr;
        _statements.getPrediction.params.Station         = stationId;
        _statements.getPrediction.params.ForecastHorizon = fcHor.count();

        for (auto& row : _db(_statements.getPrediction)) {
            // The result should be only one row
            return row.Value;
        }

        throw RuntimeError("Could not get prediction from database");
    }

    TimeSeries<double> getPredictions(const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor) override
    {
        _statements.getPredictions.params.Model           = model;
        _statements.getPredictions.params.Pollutant       = pollutantId;
        _statements.getPredictions.params.Aggregation     = aggr;
        _statements.getPredictions.params.Station         = stationId;
        _statements.getPredictions.params.ForecastHorizon = fcHor.count();

        TimeSeries<double> result;
        for (auto& row : _db(_statements.getPredictions)) {
            result.insert(from_epoch(row.Date), row.Value);
        }

        return result;
    }

    std::vector<double> getPredictionValues(chrono::date_time basetime,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor) override
    {
        _statements.getPredictionValues.params.Basetime        = to_epoch(basetime);
        _statements.getPredictionValues.params.Pollutant       = pollutantId;
        _statements.getPredictionValues.params.Aggregation     = aggr;
        _statements.getPredictionValues.params.Station         = stationId;
        _statements.getPredictionValues.params.ForecastHorizon = fcHor.count();

        std::vector<double> result;
        for (auto& row : _db(_statements.getPredictionValues)) {
            result.push_back(row.Value);
        }

        return result;
    }

    TimeSeries<double> getPredictions(chrono::date_time startDate,
        chrono::date_time endDate,
        const std::string& model,
        const std::string& stationId,
        const std::string& pollutantId,
        const std::string& aggr,
        chrono::days fcHor) override
    {
        _statements.getPredictionsInRange.params.startDate       = to_epoch(startDate);
        _statements.getPredictionsInRange.params.endDate         = to_epoch(endDate);
        _statements.getPredictionsInRange.params.Model           = model;
        _statements.getPredictionsInRange.params.Pollutant       = pollutantId;
        _statements.getPredictionsInRange.params.Aggregation     = aggr;
        _statements.getPredictionsInRange.params.Station         = stationId;
        _statements.getPredictionsInRange.params.ForecastHorizon = fcHor.count();

        TimeSeries<double> result;
        for (auto& row : _db(_statements.getPredictionsInRange)) {
            result.insert(from_epoch(row.Date), row.Value);
        }

        return result;
    }

    std::vector<std::string> getModelNames(const std::string& pollutantId, const std::string& aggr) override
    {
        std::vector<std::string> names;
        auto result = _db(select(predictions.Model).flags(sqlpp::distinct).from(predictions).where(predictions.Aggregation == aggr and predictions.Pollutant == pollutantId).order_by(predictions.Model.asc()));
        for (auto& row : result) {
            names.push_back(row.Model);
        }
        return names;
    }

private:
    void createInitialDatabase()
    {
        auto transaction = start_transaction(_db);
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
        transaction.commit();
    }

    void prepareStatements()
    {
#ifndef _WIN32
        _statements.addPrediction         = _db.prepare(addPredictionQuery());
        _statements.getPrediction         = _db.prepare(getPredictionQuery());
        _statements.getPredictions        = _db.prepare(getPredictionsQuery());
        _statements.getPredictionValues   = _db.prepare(getPredictionValuesQuery());
        _statements.getPredictionsInRange = _db.prepare(getPredictionsInRangeQuery());
#endif
    }

    ConnectionType _db;

    struct PreparedStatements
    {
        using AddPredictionQuery         = decltype(addPredictionQuery());
        using GetPredictionQuery         = decltype(getPredictionQuery());
        using GetPredictionsQuery        = decltype(getPredictionsQuery());
        using GetPredictionValuesQuery   = decltype(getPredictionValuesQuery());
        using GetPredictionsInRangeQuery = decltype(getPredictionsInRangeQuery());

        PreparedStatement<ConnectionType, AddPredictionQuery> addPrediction;
        PreparedStatement<ConnectionType, GetPredictionQuery> getPrediction;
        PreparedStatement<ConnectionType, GetPredictionsQuery> getPredictions;
        PreparedStatement<ConnectionType, GetPredictionValuesQuery> getPredictionValues;
        PreparedStatement<ConnectionType, GetPredictionsInRangeQuery> getPredictionsInRange;
    };

    PreparedStatements _statements;
};

inline sqlpp::sqlite3::connection_config createSqliteConfig(std::string_view filename)
{
    sqlpp::sqlite3::connection_config config;
    config.path_to_database = filename;
    config.flags            = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

#ifdef DEBUG_QUERIES
    config.debug = true;
#endif

    return config;
}

using SqlitePredictionDatabase = PredictionDatabase<sqlpp::sqlite3::connection>;

#ifdef OPAQ_ENABLE_POSTGRESQL
inline std::shared_ptr<sqlpp::postgresql::connection_config> createPostgresConfig(std::string_view host, std::string_view user, std::string_view pass)
{
    auto config      = std::make_shared<sqlpp::postgresql::connection_config>();
    config->host     = host;
    config->user     = user;
    config->password = pass;
    config->dbname   = "opaqdb";

#ifdef DEBUG_QUERIES
    config->debug = true;
#endif

    return config;
}

using PostgresPredictionDatabase = PredictionDatabase<sqlpp::postgresql::connection>;
#endif
}
