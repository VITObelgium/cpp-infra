#pragma once

#include "Logger.h"
#include "DateTime.h"
#include "Component.h"
#include "Pollutant.h"
#include "Aggregation.h"

namespace opaq
{

// forward declaration
class Component;
class IGridProvider;
class IGridProvider;
class MeteoProvider;
class DataProvider;
class ForecastBuffer;
class IMappingBuffer;
class AQNetworkProvider;

class Model : public Component
{
public:
    Model(const std::string& name);

    void setBaseTime(const chrono::date_time& baseTime);
    void setPollutant(const Pollutant& pollutant);
    void setAggregation(Aggregation::Type aggr);
    void setForecastHorizon(chrono::days forecastHorizon);
    void setAQNetworkProvider(AQNetworkProvider& aqNetworkProvider);
    void setGridProvider(IGridProvider& gridProvider);
    void setInputProvider(DataProvider& input);
    void setMeteoProvider(MeteoProvider* meteo);
    void setBuffer(ForecastBuffer* buffer);
    void setMappingBuffer(IMappingBuffer& buffer);

    virtual void run() = 0;

    int getNoData() const noexcept;
    void setNoData(int missing);

protected:
    chrono::date_time getBaseTime();
    const Pollutant& getPollutant();
    Aggregation::Type getAggregation();
    chrono::days getForecastHorizon();

    AQNetworkProvider& getAQNetworkProvider();
    IGridProvider& getGridProvider();
    DataProvider& getInputProvider();
    MeteoProvider* getMeteoProvider();
    ForecastBuffer* getBuffer();
    IMappingBuffer& getMappingBuffer();

    Logger _logger;

private:
    chrono::date_time _baseTime;    //< run for this basetime
    Pollutant _pollutant;           //< run for this pollutant
    Aggregation::Type _aggregation; //< run for this aggregation
    chrono::days _forecastHorizon;  //< maximum forecast horizon to run to

    AQNetworkProvider* _aqNetworkProvider;
    IGridProvider* _gridProvider;
    DataProvider* _input;
    MeteoProvider* _meteo;
    ForecastBuffer* _buffer;
    IMappingBuffer* _mappingBuffer;

    int _missing_value; //!< missing value, can be set in configuration, default set here
};

}
