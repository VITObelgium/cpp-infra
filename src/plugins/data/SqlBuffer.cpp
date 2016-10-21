#include "SqlBuffer.h"

#include "PredictionDatabase.h"

namespace OPAQ
{

static const char* s_noData = "n/a";

SqlBuffer::SqlBuffer()
: _logger("SqlBuffer")
, _noData(-9999)
, _baseTimeSet(false)
{
}

SqlBuffer::~SqlBuffer() = default;

void SqlBuffer::configure(TiXmlElement* configuration, const std::string& componentName, IEngine&)
{
    setName(componentName);

    if (!configuration)
        throw NullPointerException("No configuration element given for SqlBuffer...");

    // parse filename
    auto filename = XmlTools::getChildValue<std::string>(configuration, "filename");

    // need to specify the time interval for which to store these values...
    //    this has to be generic, the baseTime resolution can be different from the
    //    forecast time resolution...
    _baseTimeResolution = std::chrono::hours(XmlTools::getChildValue(configuration, "basetime_resolution", 24));
    _fcTimeResolution   = std::chrono::hours(XmlTools::getChildValue(configuration, "fctime_resolution", 24));
    _db                 = std::make_unique<PredictionDatabase>(filename);
}

void SqlBuffer::setNoData(double noData)
{
    this->_noData = noData;
}

void SqlBuffer::throwIfNotConfigured() const
{
    if (!_db)
    {
        throw RunTimeException("SqlBuffer Not fully configured");
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
    auto fh = std::chrono::floor<chrono::days>(std::chrono::seconds((forecast.firstDateTime() - baseTime) / std::chrono::seconds(_fcTimeResolution)));

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
    throw RunTimeException("not sure what to return here, need extra information ???");
}

TimeSeries<double> SqlBuffer::getValues(const chrono::date_time& baseTime,
                                        const std::vector<chrono::days>& fc_hor, const std::string& stationId,
                                        const std::string& pollutantId, Aggregation::Type aggr)
{
    throw RunTimeException("IMPLEMENT ME !!");
}

// return hindcast vector of model values for a fixed forecast horizon
// for a forecasted day interval
TimeSeries<double> SqlBuffer::getValues(chrono::days fcHor,
                                        const chrono::date_time& fcTime1, const chrono::date_time& fcTime2,
                                        const std::string& stationId, const std::string& pollutantId,
                                        Aggregation::Type aggr)
{
    throwIfNotConfigured();

    auto t1 = fcTime1 - fcHor;
    auto t2 = fcTime2 - fcHor;

    auto aggStr = Aggregation::getName(aggr);
    auto result = _db->getPredictions(t1, t2, _currentModel, stationId, pollutantId, aggStr, fcHor);

    if (result.isEmpty())
    {
        for (auto fct = t1; fct <= t2; fct += _baseTimeResolution)
        {
            result.insert(fct, _noData);
        }
    }
    else
    {
        // fill up with nodata
        auto firstDate = result.firstDateTime();
        for (auto fct = t1; fct < firstDate; fct += _baseTimeResolution)
        {
            result.insert(fct, _noData);
        }

        for (auto fct = result.lastDateTime() + _baseTimeResolution; fct <= t2; fct += _baseTimeResolution)
        {
            result.insert(fct, _noData);
        }
    }

    auto secs = std::chrono::seconds(_baseTimeResolution);
    std::cout << result.size() << " <-> " << ((fcTime2 - fcTime1) / secs) + 1 << std::endl;
    assert(result.size() == ((fcTime2 - fcTime1) / secs) + 1);

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

    auto aggStr = Aggregation::getName(aggr);
    auto results = _db->getPredictionValues(baseTime, stationId, pollutantId, aggStr, fcHor);
    if (results.empty())
    {
        results = std::vector<double>(getModelNames(pollutantId, aggr).size(), getNoData());
    }

    return results;
}

std::vector<std::string> SqlBuffer::getModelNames(const std::string& pollutantId, Aggregation::Type aggr)
{
    throwIfNotConfigured();
    return _db->getModelNames(pollutantId, Aggregation::getName(aggr));
}
}

OPAQ_REGISTER_PLUGIN(OPAQ::SqlBuffer);
