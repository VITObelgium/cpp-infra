/*
 * Plugin.h
 *
 *  Created on: Jan 9, 2014
 *      Author: bino.maiheu@vito.be
 */

#pragma once

#include <ratio>
#include <string>
#include <vector>
#include <chrono>

#include "../Exceptions.h"
#include "../tools/ExceptionTools.h"
#include "../DateTime.h"

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
    const OPAQ::Config::Component& getValues() const
    {
        throwOnNullPtr(_values);
        return *_values;
    }

    void setValues(const OPAQ::Config::Component& values)
    {
        _values = &values;
    }

    // Throws OPAQ::NullPointerException
    const OPAQ::Config::Component& getMeteo() const
    {
        throwOnNullPtr(_meteo);
        return *_meteo;
    }

    void setMeteo(const OPAQ::Config::Component& meteo)
    {
        _meteo = &meteo;
    }

    void resetMeteo()
    {
        _meteo = nullptr;
    }

    // Throws OPAQ::NullPointerException
    const OPAQ::Config::Component& getBuffer() const
    {
        throwOnNullPtr(_buffer);
        return *_buffer;
    }

    void setBuffer(const OPAQ::Config::Component& buffer)
    {
        _buffer = &buffer;
    }

    void resetBuffer()
    {
        _buffer = nullptr;
    }

    // Throws OPAQ::NullPointerException
    const OPAQ::Config::Component& getOutputWriter() const
    {
        throwOnNullPtr(_outputWriter);
        return *_outputWriter;
    }

    void setOutputWriter(const Component& ow)
    {
        _outputWriter = &ow;
    }

    void resetOutputWriter()
    {
        _outputWriter = nullptr;
    }

    void addModel(const Component& model)
    {
        _models.push_back(model);
    }

    // returns a list of models used in the forecast...
    const std::vector<Component>& getModels() { return _models; }

    /** Set the requested forecast horizon */
    void setHorizon(chrono::days fcHor) { _fcHor = fcHor; }

    /** Returns the requested (max) forecast horizon for the forecasts */
    chrono::days getHorizon() { return _fcHor; }

private:
    // vector of models to run in the forecast
    std::vector<Component> _models;

    // input data provider components
    const Component* _values;
    const Component* _meteo;

    // forecast buffer component
    const Component* _buffer;

    // output writer component
    const Component* _outputWriter;

    chrono::days _fcHor; //!< requested max forecast horizon
};

}
}
