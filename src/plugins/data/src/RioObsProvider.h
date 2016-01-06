/*
 * RioObsProvider.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef RIOOBSPROVIDER_H_
#define RIOOBSPROVIDER_H_

#include <opaq.h>
#include <fstream>
#include <algorithm> 	// std::transform
#include <string>

namespace OPAQ {

class RioObsProvider: public OPAQ::DataProvider {
public:
  RioObsProvider();
  virtual ~RioObsProvider();
  
  LOGGER_DEC();
  
  static const std::string POLLUTANT_PLACEHOLDER;
  
  // OPAQ::Component methods
  
  /**
   * RioObsProvider configuration:
   *
   * <file_pattern>/full/path/to/data/file/name_of_file_for_pollutant_%POL%_.txt</file_pattern>
   */

  virtual void configure(TiXmlElement * configuration)
    throw (BadConfigurationException);
  
  // OPAQ::DataProvider methods
  virtual TimeInterval getTimeResolution();

  virtual double getNoData();

  /**
   * Simply return the observations for the station & pollutant Id between the given dates
   * the aggregation type returns what aggregation to give, default (when the 4th argument is
   * omitted, the base resolution is returned)
   *
   * The method relies on the select method of the TimeSeries template class which explicitly
   * fills up the requested series. The timestep is set based upon the aggregation time
   */
  virtual OPAQ::TimeSeries<double> getValues( const DateTime& t1, const DateTime& t2,
		  const std::string& stationId, const std::string& pollutantId,
		  OPAQ::Aggregation::Type aggr = OPAQ::Aggregation::None );
  
private:
  double              _noData;
  TimeInterval        _timeResolution;
  std::string         _pattern;
  bool                _configured;

  unsigned int        _nvalues; //< number of values on a line

  // order of the map :
  // pollutant -> ( aggregation -> ( station -> data ) )

  // buffer for the aggregations in the RIO files, includes the raw as well
  // note that in the RIO files, these aggregations are pre-calculated, but this
  // does not necessarily have to be the case...
  // station --> pollutant --> aggregation --> data
  std::map< std::string, std::map<OPAQ::Aggregation::Type, std::map<std::string, OPAQ::TimeSeries<double> > > >_buffer; //< the data buffer for the aggregations

  OPAQ::TimeSeries<double> *_getTimeSeries( const std::string& pollutant,
		  const std::string& station, OPAQ::Aggregation::Type aggr );
  
  void _readFile ( const std::string & pollutant );
  
  OPAQ::TimeSeries<double> *_getOrInitValues( const std::string & pollutant,
		  OPAQ::Aggregation::Type aggr, const std::string & station );
};
  
} /* namespace OPAQ */
#endif /* RIOOBSPROVIDER_H_ */
