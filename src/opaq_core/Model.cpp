#include "Model.h"

#include "tools/ExceptionTools.h"

namespace opaq
{

Model::Model()
: _aggregation(Aggregation::None)
, _aqNetworkProvider(0)
, _gridProvider(0)
, _input(0)
, _meteo(0)
, _buffer(0)
, _missing_value(-9999)
{
}

void Model::setBaseTime(const chrono::date_time& baseTime)
{
    _baseTime = baseTime;
}

void Model::setPollutant(const Pollutant& pollutant)
{
    _pollutant = pollutant;
}

void Model::setAggregation(Aggregation::Type aggr)
{
    _aggregation = aggr;
}

void Model::setForecastHorizon(chrono::days forecastHorizon)
{
    _forecastHorizon = forecastHorizon;
}

void Model::setAQNetworkProvider(AQNetworkProvider& aqNetworkProvider)
{
    _aqNetworkProvider = &aqNetworkProvider;
}

void Model::setGridProvider(IGridProvider* gridProvider)
{
    _gridProvider = gridProvider;
}

void Model::setInputProvider(DataProvider* input)
{
    _input = input;
}

void Model::setMeteoProvider(MeteoProvider* meteo)
{
    _meteo = meteo;
}

void Model::setBuffer(ForecastBuffer* buffer)
{
    _buffer = buffer;
}

void Model::setStationInfoProvider(IStationInfoProvider& provider)
{
    _stationInfoProvider = &provider;
}

chrono::date_time Model::getBaseTime()
{
    return _baseTime;
}

const Pollutant& Model::getPollutant()
{
    return _pollutant;
}

Aggregation::Type Model::getAggregation()
{
    return _aggregation;
}

chrono::days Model::getForecastHorizon()
{
    return _forecastHorizon;
}

AQNetworkProvider& Model::getAQNetworkProvider()
{
    throwOnNullPtr(_aqNetworkProvider);
    return *_aqNetworkProvider;
}

IGridProvider* Model::getGridProvider()
{
    throwOnNullPtr(_gridProvider);
    return _gridProvider;
}

DataProvider* Model::getInputProvider()
{
    throwOnNullPtr(_input);
    return _input;
}

MeteoProvider* Model::getMeteoProvider()
{
    throwOnNullPtr(_meteo);
    return _meteo;
}

ForecastBuffer* Model::getBuffer()
{
    throwOnNullPtr(_buffer);
    return _buffer;
}

IStationInfoProvider& Model::getStationInfoProvider()
{
    throwOnNullPtr(_stationInfoProvider);
    return *_stationInfoProvider;
}

int Model::getNoData() const noexcept
{
    return _missing_value;
}

void Model::setNoData(int missing)
{
    _missing_value = missing;
}

}
