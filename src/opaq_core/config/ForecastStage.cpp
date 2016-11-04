#include "ForecastStage.h"

#include "../Exceptions.h"
#include "../tools/ExceptionTools.h"

namespace opaq
{
namespace config
{

ForecastStage::ForecastStage()
: _values(nullptr)
, _meteo(nullptr)
, _buffer(nullptr)
, _outputWriter(nullptr)
{
}

const config::Component& ForecastStage::getValues() const
{
    throwOnNullPtr(_values);
    return *_values;
}

void ForecastStage::setValues(const config::Component& values)
{
    _values = &values;
}

const config::Component& ForecastStage::getMeteo() const
{
    throwOnNullPtr(_meteo);
    return *_meteo;
}

void ForecastStage::setMeteo(const config::Component& meteo)
{
    _meteo = &meteo;
}

const config::Component& ForecastStage::getBuffer() const
{
    throwOnNullPtr(_buffer);
    return *_buffer;
}

void ForecastStage::setBuffer(const config::Component& buffer)
{
    _buffer = &buffer;
}

const config::Component& ForecastStage::getOutputWriter() const
{
    throwOnNullPtr(_outputWriter);
    return *_outputWriter;
}

void ForecastStage::setOutputWriter(const Component& ow)
{
    _outputWriter = &ow;
}

void ForecastStage::addModel(const Component& model)
{
    _models.push_back(model);
}

const std::vector<Component>& ForecastStage::getModels() const noexcept
{
    return _models;
}

void ForecastStage::setHorizon(chrono::days fcHor)
{
    _fcHor = fcHor;
}

chrono::days ForecastStage::getHorizon() const noexcept
{
    return _fcHor;
}

}
}
