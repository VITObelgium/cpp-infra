#include "ForecastStage.h"

#include "../Exceptions.h"
#include "../tools/ExceptionTools.h"

namespace opaq
{
namespace config
{

ForecastStage::ForecastStage(chrono::days fcHor,
                             config::Component values,
                             config::Component meteo,
                             config::Component buffer,
                             config::Component outputWriter,
                             std::vector<Component> models)
: _fcHor(fcHor)
, _values(std::move(values))
, _meteo(std::move(meteo))
, _buffer(std::move(buffer))
, _outputWriter(std::move(outputWriter))
, _models(std::move(models))
{
}

config::Component ForecastStage::getValues() const
{
    return _values;
}

config::Component ForecastStage::getMeteo() const
{
    return _meteo;
}

config::Component ForecastStage::getBuffer() const
{
    return _buffer;
}

config::Component ForecastStage::getOutputWriter() const
{
    return _outputWriter;
}

const std::vector<Component>& ForecastStage::getModels() const noexcept
{
    return _models;
}

chrono::days ForecastStage::getHorizon() const noexcept
{
    return _fcHor;
}

}
}
