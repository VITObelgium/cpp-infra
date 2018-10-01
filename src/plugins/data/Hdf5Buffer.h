/*
 * H5ForecastStore.h
 *
 */

#ifndef Hdf5Buffer_H_
#define Hdf5Buffer_H_

#include "data/ForecastOutputWriter.h"

#include <H5Cpp.h>

namespace opaq {

class Hdf5Buffer : public ForecastBuffer
{
public:
    Hdf5Buffer();

    static std::string name();

    // ==================================================
    // OPAQ::Component methods
    // ==================================================
    /**
   * <!-- created if it does not exist, appended if it does -->
   * <filename>/path/to/datafile.h5</filename>
   * <!-- offset in days relative to the base time indicating
   *   the earliest day for which data will be stored
   *   if a new file is created, this will be the first date in the file
   *   when an existing file is opened, this cannot point to a date
   *   before the first date in the file
   *   for example: if base time is 2014/01/10 and offset = -10,
   *    the first date stored is 2013/12/31
   * <offset>-10</offset>
   * throws BadConfigurationException
   */
    void configure(const inf::ConfigNode& configuration, const std::string& componentName, IEngine& engine) override;

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

    // the current model is already set by the DataProvider parent class

    // ==================================================
    // OPAQ::ForecastBuffer methods
    // ==================================================

    virtual std::vector<std::string> getModelNames(const std::string& pollutantId, Aggregation::Type aggr) override;

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
    TimeSeries<double> getForecastValues(const chrono::date_time& baseTime,
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
    TimeSeries<double> getForecastValues(chrono::days fc_hor,
        const chrono::date_time& fcTime1,
        const chrono::date_time& fcTime2,
        const std::string& stationId,
        const std::string& pollutantId,
        Aggregation::Type aggr) override;

    /*
  virtual std::vector<double> getValues(const TimeInterval & beginOffset,
          const TimeInterval & endOffset,
          const std::string & parameter,
          const std::string & station);


  virtual std::vector<double> getValues(const std::string & modelName,
          const TimeInterval & beginOffset,
          const TimeInterval & endOffset,
          const std::string & parameter,
          const std::string & station );


  virtual std::vector<double> getValues(const TimeInterval & beginOffset,
          const TimeInterval & endOffset,
          const std::string & parameter,
          const std::string & station,
          const ForecastHorizon & forecastHorizon);

  virtual std::vector<double> getValues(const std::string & modelName,
          const TimeInterval & beginOffset,
          const TimeInterval & endOffset,
          const std::string & parameter,
          const std::string & station,
          const ForecastHorizon & forecastHorizon );

  virtual std::vector<double> getValues(const std::string & parameter,
          const TimeInterval & offset = TimeInterval(0),
          const ForecastHorizon & forecastHorizon = ForecastHorizon(0));
*/

    // OPAQ::DataBuffer methods
    virtual void setNoData(double noData) override;

    /**
   * throws Hdf5BufferRunTimeException on runtime errors:
   * . forecast horizons that don't lie on exact day boundaries
   * . number of values != number of forecast horizons
   */

    /*
  virtual void setValues(const std::string &modelName,
       const std::vector<double> & values,
       const std::vector<ForecastHorizon> & forecastHorizons,
       const std::string & parameter, const std::string & station);
   */

    void setForecastHorizon(chrono::days fcHor) override;

private:
    std::chrono::seconds getBaseTimeResolutionInSeconds();
    void throwIfNotConfigured() const;

    void createFile(const std::string& filename);
    void openFile(const std::string& filename);

    //  int _getIndexInStringDataSet (H5::DataSet & dataSet, const std::string &parameter);
    //  unsigned int _getStringDataSetSize (H5::DataSet & dataSet);
    //  void _addToStringDataSet (H5::DataSet & dataSet, const std::string & value);

    std::unique_ptr<H5::H5File> _h5file; //!< HDF5 file handle for the buffer file

    H5::DataSet _parametersSet, _stationsSet;
    H5::StrType _stringType;

    double _noData;
    // int      _offset;

    std::chrono::hours _baseTimeResolution; //! the time resolution at which to store basetimes
    std::chrono::hours _fcTimeResolution;   //! the time resolution at which to store the forecast values

    chrono::date_time _startDate; //!< the start stored in the file (cannot add values before it)

    chrono::date_time _baseTime; //!< the basetime against which to offset the intervals given by the
                                 //!< getValues and setValues routines

    chrono::days _fcHor;
};

} /* namespace OPAQ */
#endif /* HDF5FORECASTSSTORE_H_ */
