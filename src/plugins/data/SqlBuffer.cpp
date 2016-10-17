#include "SqlBuffer.h"

#include "PredictionDatabase.h"

namespace OPAQ
{

static const char* s_noData = "n/a";

SqlBuffer::SqlBuffer()
: _logger("Hdf5Buffer")
, _noData(-9999)
, _baseTimeSet(false)
{
}

SqlBuffer::~SqlBuffer() = default;

void SqlBuffer::configure(TiXmlElement* configuration, IEngine&)
{
    if (!configuration)
        throw NullPointerException("No configuration element given for Hdf5Buffer...");

    // parse filename
    auto* fileEl = configuration->FirstChildElement("filename");
    if (!fileEl)
        throw BadConfigurationException("filename element not found");
    
    auto filename = fileEl->GetText();

    // need to specify the time interval for which to store these values...
    //    this has to be generic, the baseTime resolution can be different from the
    //    forecast time resolution...
    auto* baseResEl = configuration->FirstChildElement("basetime_resolution");
    if (!baseResEl)
    {
        _baseTimeResolution = TimeInterval(24, TimeInterval::Hours);
    }
    else
    {
        _baseTimeResolution = TimeInterval(atoi(baseResEl->GetText()), TimeInterval::Hours);
    }

    auto* fcResEl = configuration->FirstChildElement("fctime_resolution");
    if (!fcResEl)
    {
        _fcTimeResolution = TimeInterval(24, TimeInterval::Hours);
    }
    else
    {
        _fcTimeResolution = TimeInterval(atoi(fcResEl->GetText()), TimeInterval::Hours);
    }

    _db = std::make_unique<PredictionDatabase>(filename);
}

void SqlBuffer::setNoData(double noData)
{
    this->_noData = noData;
}

double SqlBuffer::getNoData()
{
    return _noData;
}

void SqlBuffer::setValues(const DateTime& baseTime,
                          const OPAQ::TimeSeries<double>& forecast,
                          const std::string& stationId,
                          const std::string& pollutantId,
                          OPAQ::Aggregation::Type aggr)
{
    auto aggStr = Aggregation::getName(aggr);
    for (size_t i = 0; i < forecast.size(); ++i)
    {
        // TODO: two missing values
        _db->addPrediction(baseTime.getUnixTime(), forecast.datetime(i).getUnixTime(), "", stationId, pollutantId, aggStr, 0, forecast.value(i));
    }
}

TimeInterval SqlBuffer::getTimeResolution()
{
    return _fcTimeResolution;
}

TimeInterval SqlBuffer::getBaseTimeResolution()
{
    return _baseTimeResolution;
}

OPAQ::TimeSeries<double> SqlBuffer::getValues(const DateTime& t1,
                                               const DateTime& t2,
                                               const std::string& stationId,
                                               const std::string& pollutantId,
                                               OPAQ::Aggregation::Type aggr)
{
    throw RunTimeException("not sure what to return here, need extra information ???");

    OPAQ::TimeSeries<double> out;
    return out;
}

OPAQ::TimeSeries<double> SqlBuffer::getValues(const DateTime& baseTime,
                                               const std::vector<OPAQ::TimeInterval>& fc_hor, const std::string& stationId,
                                               const std::string& pollutantId, OPAQ::Aggregation::Type aggr)
{
    throw RunTimeException("IMPLEMENT ME !!");

    OPAQ::TimeSeries<double> out;
    return out;
}

// return hindcast vector of model values for a fixed forecast horizon
// for a forecasted day interval
OPAQ::TimeSeries<double> SqlBuffer::getValues(const OPAQ::TimeInterval fc_hor,
                                               const DateTime& fcTime1, const DateTime& fcTime2,
                                               const std::string& stationId, const std::string& pollutantId,
                                               OPAQ::Aggregation::Type aggr)
{
    return {};
}

// return model values for a given baseTime / forecast horizon
// storage in file : "model x station x baseTime x fcHorizon"
std::vector<double> SqlBuffer::getModelValues(const DateTime& baseTime, const OPAQ::TimeInterval& fc_hor,
                                               const std::string& stationId, const std::string& pollutantId, OPAQ::Aggregation::Type aggr)
{
    return {};
}

std::vector<std::string> SqlBuffer::getModelNames(const std::string& pollutantId, OPAQ::Aggregation::Type aggr)
{
    return {};
}

}

OPAQ_REGISTER_PLUGIN(OPAQ::SqlBuffer);
