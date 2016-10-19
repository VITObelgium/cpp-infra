/*
 * ForecastOutputWriter.h
 *
 * Bino Maiheu
 */

#pragma once

#include <string>

#include "../AQNetworkProvider.h"
#include "../Aggregation.h"
#include "../Component.h"
#include "../DateTime.h"
#include "ForecastBuffer.h"

namespace OPAQ
{

class Pollutant;

class ForecastOutputWriter : public Component
{

public:
    ForecastOutputWriter();

    virtual void write(Pollutant* pol, Aggregation::Type aggr, const DateTime& baseTime) = 0;

    // some setters
    void setAQNetworkProvider(AQNetworkProvider& n) { _net = &n; }
    void setBuffer(ForecastBuffer* buf) { _buf = buf; }
    void setForecastHorizon(days fc) { _fcHor = fc; }

protected:
    ForecastBuffer* getBuffer() { return _buf; }
    AQNetworkProvider* getAQNetworkProvider() { return _net; }
    days getForecastHorizon() { return _fcHor; }

private:
    AQNetworkProvider* _net;
    ForecastBuffer* _buf;
    days _fcHor;
};

}
