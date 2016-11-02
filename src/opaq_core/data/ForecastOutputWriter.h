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
    virtual void write(const Pollutant& pol, Aggregation::Type aggr, const chrono::date_time& baseTime) = 0;

    // some setters
    void setAQNetworkProvider(AQNetworkProvider& n) { _net = &n; }
    void setBuffer(ForecastBuffer* buf) { _buf = buf; }
    void setForecastHorizon(chrono::days fc) { _fcHor = fc; }

protected:
    ForecastBuffer* getBuffer() { return _buf; }
    AQNetworkProvider* getAQNetworkProvider() { return _net; }
    chrono::days getForecastHorizon() { return _fcHor; }

private:
    AQNetworkProvider* _net = nullptr;
    ForecastBuffer* _buf = nullptr;
    chrono::days _fcHor;
};

}
