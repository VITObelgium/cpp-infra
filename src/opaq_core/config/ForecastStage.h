/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: bino.maiheu@vito.be
 */

#pragma once

#include <string>
#include <vector>

#include "../Exceptions.h"
#include "../tools/ExceptionTools.h"
#include "../TimeInterval.h"
#include "Component.h"

namespace OPAQ
{
namespace Config
{

/**
   * Forecast configuration class
   */
class ForecastStage
{
public:
    ForecastStage();

    /**
   *  Returns the dataprovider for the observed concentration values
   *  Throws OPAQ::NullPointerException
   */
    OPAQ::Config::Component& getValues() const
    {
        throwOnNullPtr(_values);
        return *_values;
    }

    void setValues(OPAQ::Config::Component& values)
    {
        _values = &values;
    }

    // Throws OPAQ::NullPointerException
    OPAQ::Config::Component& getMeteo() const
    {
        throwOnNullPtr(_meteo);
        return *_meteo;
    }

    void setMeteo(OPAQ::Config::Component* meteo)
    {
        _meteo = meteo;
    }

    // Throws OPAQ::NullPointerException
    OPAQ::Config::Component& getBuffer() const
    {
        throwOnNullPtr(_buffer);
        return *_buffer;
    }

    void setBuffer(OPAQ::Config::Component* buffer)
    {
        _buffer = buffer;
    }

    // Throws OPAQ::NullPointerException
    OPAQ::Config::Component& getOutputWriter() const
    {
        throwOnNullPtr(_outputWriter);
        return *_outputWriter;
    }

    void setOutputWriter(Component* ow)
    {
        _outputWriter = ow;
    }

    // returns a list of models used in the forecast...
    std::vector<Component*>& getModels() { return models; }

    /** Set the requested forecast horizon */
    void setHorizon(const TimeInterval& f) { _fcHor = f; }

    /** Returns the requested (max) forecast horizon for the forecasts */
    TimeInterval& getHorizon() { return _fcHor; }

private:
    // vector of models to run in the forecast
    std::vector<Component*> models;

    // input data provider components
    Component* _values;
    Component* _meteo;

    // forecast buffer component
    Component* _buffer;

    // output writer component
    Component* _outputWriter;

    TimeInterval _fcHor; //!< requested max forecast horizon
};

}
}
