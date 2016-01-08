/*
 * H5ForecastStore.h
 *
 */

#ifndef Hdf5Buffer_H_
#define Hdf5Buffer_H_

#include <opaq.h>
#include <H5Cpp.h>

namespace OPAQ {

class Hdf5Buffer: public OPAQ::ForecastBuffer {
public:
  Hdf5Buffer();
  virtual ~Hdf5Buffer();
  
  LOGGER_DEC();

  static const std::string START_DATE_NAME;        // attribute that holds the start date
  static const std::string FORECAST_DATASET_NAME;
  static const std::string BASETIME_DATASET_NAME;
  static const std::string DIMENSIONS_NAME;
  static const std::string DIMENSIONS;
  static const std::string DESCRIPTION_NAME;
  static const std::string DESCRIPTION;
  static const std::string MODELS_DATASET_NAME;
  static const std::string STATION_DATASET_NAME;

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
   */
  virtual void configure(TiXmlElement * configuration)
    throw (BadConfigurationException);

  // ==================================================
  // OPAQ::DataProvider methods
  // ==================================================
  /**
   * Returns the time resolution of the Hdf5Buffer, this returns
   * a TimeInterval object of 1 day. 
   */
  virtual TimeInterval getTimeResolution();

  virtual TimeInterval getBaseTimeResolution();

  virtual double getNoData();

  virtual OPAQ::TimeSeries<double> getValues( const DateTime& t1,
  		  	  	  	  	  	  	  	  	  	  const DateTime& t2,
  											  const std::string& stationId,
  											  const std::string& pollutantId,
											  OPAQ::Aggregation::Type aggr = OPAQ::Aggregation::None );

  // the current model is already set by the DataProvider parent class

  // ==================================================
  // OPAQ::ForecastBuffer methods
  // ==================================================

  virtual std::vector<std::string> getModelNames( const std::string& pollutantId, OPAQ::Aggregation::Type aggr );

  /**
   * Fill the Hdf5 file with the values given by the current basetime & the forecast
   * horizon
   */
  virtual void setValues( const DateTime &baseTime,
 		  	  	  	  	  const OPAQ::TimeSeries<double>& forecast,
 						  const std::string& stationId,
 						  const std::string& pollutantId,
 						  OPAQ::Aggregation::Type aggr );


  /**
   * Return all the model values for a given baseTime and forecast horizon. The given current model
   * which is set in the DataProvider parent class is ignored here...
   */
  virtual std::vector<double> getModelValues( const DateTime &baseTime,
		                                      const OPAQ::TimeInterval& fc_hor,
										      const std::string& stationId,
										      const std::string& pollutantId,
										      OPAQ::Aggregation::Type aggr );


  /**
    * This routine retrieves the forecasted values for a specific base time
    * as a function of forecast horizon, given by the vector of time intervals
    */
   virtual OPAQ::TimeSeries<double> getValues( const DateTime &baseTime,
  											  const std::vector<OPAQ::TimeInterval>& fc_hor,
  											  const std::string& stationId,
  											  const std::string& pollutantId,
  											  OPAQ::Aggregation::Type aggr );

  /**
   * This one gives the forecasts between the forecast times1 and 2 for a given fixed time lag (the
   * fc_hor. This routine can be used to e.g. retieve the archived day+2 forecasts for a given period
   * to e.g. calculate real time corrections. The user needs to be avare that the two DateTimes given
   * are really the forecast times (so the datetimes for which the forecast is intended
   */
   virtual OPAQ::TimeSeries<double> getValues( const OPAQ::TimeInterval fc_hor,
           	  	  	  	  	  	  	  	  	  const DateTime &fcTime1,
 											  const DateTime &fcTime2,
 											  const std::string& stationId,
 											  const std::string& pollutantId,
 											  OPAQ::Aggregation::Type aggr );

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
  virtual void setNoData(double noData);
  
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


private:
  std::string _filename;  //!< filename for the buffer file
  H5::H5File *_h5file;    //!< HDF5 file handle for the buffer file

  H5::DataSet _dataSet, _parametersSet, _stationsSet;

  double   _noData;
  // int      _offset;

  TimeInterval _baseTimeResolution; //! the time resolution at which to store basetimes
  TimeInterval _fcTimeResolution;   //! the time resolution at which to store the forecast values

  DateTime _startDate; //!< the start stored in the file (cannot add values before it)

  DateTime _baseTime;  //!< the basetime against which to offset the intervals given by the 
                       //!< getValues and setValues routines

  bool     _configured;  //!< Flag, true if the OPAQ::Component configuration went well
  bool     _baseTimeSet; //!< Flag, true if a basetime was given to the 
  
private:
  void _closeFile();
  
  /**
   * Checks whether the basetime is set and the configuration is succesful
   */
  void _checkFullyConfigured();
  /**
   * Checks whether the file exists and open it
   * Note:: we always ope in RDWR mode, even only for read only operations...
   *        this might be a bit dangerous... 
   */
  void _checkIfExistsAndOpen();

  void _createOrOpenFile() throw (BadConfigurationException);
  
  void _createFile(const std::string & filename);
  
  void _openFile(const std::string &filename)
    throw (BadConfigurationException);
  
  //	int _getIndexInStringDataSet (H5::DataSet & dataSet, const std::string &parameter);
  //	unsigned int _getStringDataSetSize (H5::DataSet & dataSet);
  //	void _readStringData (char ** buffer, H5::DataSet & dataSet);
  //	void _addToStringDataSet (H5::DataSet & dataSet, const std::string & value);
};
  
} /* namespace OPAQ */
#endif /* HDF5FORECASTSSTORE_H_ */
