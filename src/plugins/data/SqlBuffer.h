#pragma once

#include "PredictionDatabase.h"
#include "data/ForecastBuffer.h"

#include <memory>

namespace sqlpp
{
namespace sqlite3
{
class connection;
}
}

namespace OPAQ
{

class PredictionDatabase;

class SqlBuffer : public ForecastBuffer
{
public:
    SqlBuffer();
    virtual ~SqlBuffer();

    static std::string name();

    // throws BadConfigurationException
    void configure(TiXmlElement* configuration, const std::string& componentName, IEngine& engine) override;

    // ==================================================
    // OPAQ::DataProvider methods
    // ==================================================
    /**
   * Returns the time resolution of the Hdf5Buffer, this returns
   * a TimeInterval object of 1 day.
   */
    virtual std::chrono::hours getTimeResolution() override;
    virtual std::chrono::hours getBaseTimeResolution() override;

    virtual double getNoData() override;
    virtual TimeSeries<double> getValues(const chrono::date_time& t1,
                                         const chrono::date_time& t2,
                                         const std::string& stationId,
                                         const std::string& pollutantId,
                                         Aggregation::Type aggr = Aggregation::None) override;

    // ==================================================
    // OPAQ::ForecastBuffer methods
    // ==================================================

    virtual std::vector<std::string> getModelNames(const std::string& pollutantId, OPAQ::Aggregation::Type aggr) override;

    /**
   * Fill the Hdf5 file with the values given by the current basetime & the forecast
   * horizon
   */
    virtual void setValues(const chrono::date_time& baseTime,
                           const TimeSeries<double>& forecast,
                           const std::string& stationId,
                           const std::string& pollutantId,
                           Aggregation::Type aggr) override;

    /**
   * Return all the model values for a given baseTime and forecast horizon. The given current model
   * which is set in the DataProvider parent class is ignored here...
   */
    virtual std::vector<double> getModelValues(const chrono::date_time& baseTime,
                                               chrono::days fc_hor,
                                               const std::string& stationId,
                                               const std::string& pollutantId,
                                               Aggregation::Type aggr) override;

    /**
    * This routine retrieves the forecasted values for a specific base time
    * as a function of forecast horizon, given by the vector of time intervals
    */
    virtual TimeSeries<double> getValues(const chrono::date_time& baseTime,
                                         const std::vector<chrono::days>& fc_hor,
                                         const std::string& stationId,
                                         const std::string& pollutantId,
                                         Aggregation::Type aggr) override;

    /**
   * This one gives the forecasts between the forecast times1 and 2 for a given fixed time lag (the
   * fc_hor. This routine can be used to e.g. retieve the archived day+2 forecasts for a given period
   * to e.g. calculate real time corrections. The user needs to be avare that the two DateTimes given
   * are really the forecast times (so the datetimes for which the forecast is intended
   */
    virtual TimeSeries<double> getValues(chrono::days fc_hor,
                                         const chrono::date_time& fcTime1,
                                         const chrono::date_time& fcTime2,
                                         const std::string& stationId,
                                         const std::string& pollutantId,
                                         Aggregation::Type aggr) override;

    // OPAQ::DataBuffer methods
    virtual void setNoData(double noData) override;

private:
    void throwIfNotConfigured() const;

    Logger _logger;
    std::unique_ptr<PredictionDatabase> _db;

    double _noData;

    std::chrono::hours _baseTimeResolution; //! the time resolution at which to store basetimes
    std::chrono::hours _fcTimeResolution;   //! the time resolution at which to store the forecast values

    chrono::date_time _startDate; //!< the start stored in the file (cannot add values before it)
    chrono::date_time _baseTime;  //!< the basetime against which to offset the intervals given by the
                         //!< getValues and setValues routines

    bool _baseTimeSet; //!< Flag, true if a basetime was given to the
};
}
