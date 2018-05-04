#pragma once

#include "../Component.h"
#include "DataProvider.h"
#include <vector>

namespace opaq
{

class ForecastBuffer : public DataProvider
{
public:
    /**
   * Return a list of available models in the buffer
   */
    virtual std::vector<std::string> getModelNames(const std::string& pollutantId, Aggregation::Type aggr) = 0;

    /**
   * Also return the basetime resolution
   */
    virtual std::chrono::hours getBaseTimeResolution() = 0;

    /**
   * Set the nodata placeholder to use for incoming values,
   * alternatively you can also use the current nodata placeholder
   * as returned by getNoData()
   */
    virtual void setNoData(double noData) = 0;

    /**
   * Store the given values with the given forecast horizons for given
   * ids.
   *
   * Only forecasted values are stored where the forecast horizons are relative
   * with respect to the basetime set in the DataProvider from which this
   * class derives...
   *
   * This method is typically used by a forecast model to set the values
   */
    virtual void setValues(const chrono::date_time& baseTime,
                           const TimeSeries<double>& forecast,
                           const std::string& stationId,
                           const std::string& pollutantId,
                           Aggregation::Type aggr) = 0;

    /**
   * Return all the model values for a given baseTime and forecast horizon. The given current model
   * which is set in the DataProvider parent class is ignored here...
   */
    virtual std::vector<double> getModelValues(const chrono::date_time& baseTime,
                                               chrono::days fc_hor,
                                               const std::string& stationId,
                                               const std::string& pollutantId,
                                               Aggregation::Type aggr) = 0;

    /**
   * This routine retrieves the forecasted values for a specific base time
   * as a function of forecast horizon, given by the vector of time intervals
   */
    virtual TimeSeries<double> getForecastValues(const chrono::date_time& baseTime,
                                                 const std::vector<chrono::days>& fc_hor,
                                                 const std::string& stationId,
                                                 const std::string& pollutantId,
                                                 Aggregation::Type aggr) = 0;

    /**
   * This one gives the forecasts between the forecast times1 and 2 for a given fixed time lag (the
   * fc_hor. This routine can be used to e.g. retieve the archived day+2 forecasts for a given period
   * to e.g. calculate real time corrections
   */
    virtual TimeSeries<double> getForecastValues(chrono::days fc_hor,
                                                 const chrono::date_time& fcTime1,
                                                 const chrono::date_time& fcTime2,
                                                 const std::string& stationId,
                                                 const std::string& pollutantId,
                                                 Aggregation::Type aggr) = 0;

    virtual void setForecastHorizon(chrono::days fcHor) = 0;
};
}
