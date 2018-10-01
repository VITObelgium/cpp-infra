#include "SqlBuffer.h"

#include "PredictionDatabaseFactory.h"
#include "PredictionDatabaseInterface.h"
#include "infra/configdocument.h"

namespace opaq {

using namespace inf;
using namespace chrono_literals;

SqlBuffer::SqlBuffer()
: _noData(-9999)
, _fcHor(0)
{
}

SqlBuffer::~SqlBuffer() = default;

std::string SqlBuffer::name()
{
    return "sqlbuffer";
}

void SqlBuffer::configure(const inf::ConfigNode& configuration, const std::string& componentName, IEngine&)
{
    setName(componentName);

    if (!configuration) {
        throw InvalidArgument("No configuration element given for SqlBuffer...");
    }

    // need to specify the time interval for which to store these values...
    //    this has to be generic, the baseTime resolution can be different from the
    //    forecast time resolution...
    _baseTimeResolution = std::chrono::hours(configuration.child("basetime_resolution").value<int>().value_or(24));
    _fcTimeResolution   = std::chrono::hours(configuration.child("fctime_resolution").value<int>().value_or(24));

    // parse filename
    auto type     = std::string(configuration.child("dbtype").value());
    auto location = std::string(configuration.child("location").value());
    auto user     = std::string(configuration.child("dbuser").value());
    auto pass     = std::string(configuration.child("dbpass").value());
    _db           = factory::createPredictionDatabase(type, location, user, pass);
}

void SqlBuffer::setNoData(double noData)
{
    this->_noData = noData;
}

void SqlBuffer::throwIfNotConfigured() const
{
    if (!_db) {
        throw RuntimeError("SqlBuffer Not fully configured");
    }
}

double SqlBuffer::getNoData()
{
    return _noData;
}

void SqlBuffer::setValues(const chrono::date_time& baseTime,
    const TimeSeries<double>& forecast,
    const std::string& stationId,
    const std::string& pollutantId,
    Aggregation::Type aggr)
{
    throwIfNotConfigured();

    auto aggStr = Aggregation::getName(aggr);
    auto fh     = date::floor<chrono::days>(forecast.firstDateTime() - baseTime);

    _db->addPredictions(baseTime, _currentModel, stationId, pollutantId, aggStr, fh, forecast);
}

std::chrono::hours SqlBuffer::getTimeResolution()
{
    return _fcTimeResolution;
}

std::chrono::hours SqlBuffer::getBaseTimeResolution()
{
    return _baseTimeResolution;
}

TimeSeries<double> SqlBuffer::getValues(const chrono::date_time& t1,
    const chrono::date_time& t2,
    const std::string& stationId,
    const std::string& pollutantId,
    Aggregation::Type aggr)
{
    assert(_fcHor > 0_d);
    return getForecastValues(_fcHor, t1, t2, stationId, pollutantId, aggr);
}

TimeSeries<double> SqlBuffer::getForecastValues(const chrono::date_time& /*baseTime*/,
    const std::vector<chrono::days>& /*fc_hor*/, const std::string& /*stationId*/,
    const std::string& /*pollutantId*/, Aggregation::Type /*aggr*/)
{
    throw RuntimeError("IMPLEMENT ME !!");
}

// return hindcast vector of model values for a fixed forecast horizon
// for a forecasted day interval
TimeSeries<double> SqlBuffer::getForecastValues(chrono::days fcHor,
    const chrono::date_time& fcTime1, const chrono::date_time& fcTime2,
    const std::string& stationId, const std::string& pollutantId,
    Aggregation::Type aggr)
{
    throwIfNotConfigured();

    if (fcTime1 > fcTime2) {
        throw RuntimeError("requested fcTime1 is > fcTime2...");
    }

    auto aggStr = Aggregation::getName(aggr);
    auto result = _db->getPredictions(fcTime1, fcTime2, _currentModel, stationId, pollutantId, aggStr, fcHor);

    if (result.isEmpty()) {
        for (auto fct = fcTime1; fct <= fcTime2; fct += _baseTimeResolution) {
            result.insert(fct, _noData);
        }
    } else {
        // fill up with nodata
        auto firstDate = result.firstDateTime();
        for (auto fct = fcTime1; fct < firstDate; fct += _baseTimeResolution) {
            result.insert(fct, _noData);
        }

        for (auto fct = result.lastDateTime() + _baseTimeResolution; fct <= fcTime2; fct += _baseTimeResolution) {
            result.insert(fct, _noData);
        }
    }

    assert(result.size() == static_cast<size_t>((fcTime2 - fcTime1) / _baseTimeResolution) + 1);

    return result;
}

// return model values for a given baseTime / forecast horizon
std::vector<double> SqlBuffer::getModelValues(const chrono::date_time& baseTime,
    chrono::days fcHor,
    const std::string& stationId,
    const std::string& pollutantId,
    Aggregation::Type aggr)
{
    throwIfNotConfigured();

    auto aggStr  = Aggregation::getName(aggr);
    auto results = _db->getPredictionValues(baseTime, stationId, pollutantId, aggStr, fcHor);
    if (results.empty()) {
        results = std::vector<double>(getModelNames(pollutantId, aggr).size(), getNoData());
    }

    return results;
}

std::vector<std::string> SqlBuffer::getModelNames(const std::string& pollutantId, Aggregation::Type aggr)
{
    throwIfNotConfigured();
    auto names = _db->getModelNames(pollutantId, Aggregation::getName(aggr));
    if (names.empty()) {
        throw NotAvailableException("Error reading model list for {}, {}", pollutantId, Aggregation::getName(aggr));
    }

    return names;
}

void SqlBuffer::setForecastHorizon(chrono::days fcHor)
{
    _fcHor = fcHor;
}

}
