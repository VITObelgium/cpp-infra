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
    void setForecastHorizon(const TimeInterval& fc) { _fcHor = fc; }

protected:
    ForecastBuffer* getBuffer(void) { return _buf; }
    AQNetworkProvider* getAQNetworkProvider(void) { return _net; }
    const TimeInterval& getForecastHorizon(void) { return _fcHor; }

private:
    AQNetworkProvider* _net;
    ForecastBuffer* _buf;
    TimeInterval _fcHor;
};

}
