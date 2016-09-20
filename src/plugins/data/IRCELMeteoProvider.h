/*
 * IRCELMeteoProvider.h
 *
 *  Created on: 2015
 *      Author: bino.maiheu@vito.be
 */

#ifndef IRCELMETEOPROVIDER_H_
#define IRCELMETEOPROVIDER_H_

#include <map>

#include <opaq.h>

namespace OPAQ {

class IRCELMeteoProvider: public OPAQ::MeteoProvider {
public:
  IRCELMeteoProvider();
  virtual ~IRCELMeteoProvider();

  static const std::string METEO_PLACEHOLDER;
  static const std::string PARAMETER_PLACEHOLDER;
  static const std::string BASETIME_PLACEHOLDER;

  // OPAQ::Component methods
  // throws BadConfigurationException
  virtual void configure(TiXmlElement *cnf);

  // OPAQ::MeteoProvider methods

  /**
   * Ignored
   */
  virtual const TimeInterval& getTimeResolution();

  /**
   * throws an exception: use getNoData(const std::string &) instead
   */
  virtual double getNoData(const std::string & parameterId);

  /**
   * Return the values between t1 and t2 including the boundaries !
   */
  virtual OPAQ::TimeSeries<double> getValues( const DateTime & t1,
						    				  const DateTime & t2,
											  const std::string& meteoId,
											  const std::string& paramId );

private:
  Logger             _logger;
  bool               _configured;      //! is the object completely configured
  void               _checkConfig();
  OPAQ::TimeInterval _timeResolution;  //! what is the time resolution of the meteo data ?
  int                _nsteps;          //! number of steps in a line in the datafile
  OPAQ::DateTime     _bufferStartDate; //! requested buffer start date, if not given, then all file is stored
  bool               _bufferStartReq;  //! is this requested ?
  int                _backsearch;      //! how many days are we looking in the past to find data file

  // -- the dynamic meteo data buffer
  std::map<std::string, std::map<std::string, OPAQ::TimeSeries<double> > > _buffer;

  // -- the buffer with nodata values
  std::map<std::string, double> _nodata;

  // -- file pattern for reading the ECMWF txt files generated by IRCEL
  std::string _pattern;

  void _readFile( const std::string & meteoId, const std::string & parameterId );

  OPAQ::TimeSeries<double>* _getOrInit( const std::string & meteoId,
		                                const std::string &parameterId );

  const OPAQ::TimeSeries<double> & _getTimeSeries( const std::string & meteoId,
		                                           const std::string & parameterId );
};

} /* namespace OPAQ */
#endif /* IRCELMETEOPROVIDER_H_ */
