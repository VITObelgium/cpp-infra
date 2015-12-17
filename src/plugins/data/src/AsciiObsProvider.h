/*
 * AsciiObsProvider.h
 *
 *  Created on: 2014
 *      Author: Stijn.VanLooy@vito.be
 */

#ifndef ASCOBSPROVIDER_H_
#define ASCOBSPROVIDER_H_

#include <opaq.h>
#include <fstream>
#include <algorithm> 	// std::transform
#include <string>

namespace OPAQ {

  class AsciiObsProvider: public OPAQ::DataProvider {
  public:
    AsciiObsProvider();
    virtual ~AsciiObsProvider();
    
    LOGGER_DEC();

    static const std::string POLLUTANT_PLACEHOLDER;
    static const std::string STATION_PLACEHOLDER;

    // OPAQ::Component methods

    /**
     * AsciiObsProvider configuration:
     * <!-- range to buffer, offsets relative to base time
     *  expressed in hours -->
     * <range>
     * 	<begin_offset>-48</begin_offset>
     * 	<end_offset>0</end_offset>
     * 	<clear_on_new_basetime>false</clear_on_new_basetime>	<!-- true or false, optional: default = true -->
     * </range>
     * <file_pattern>/full/path/to/data/file/name_of_file_for_pollutant_%POL%_.txt</file_pattern>
     */
    virtual void configure(TiXmlElement * configuration)
      throw (BadConfigurationException);
    
    // OPAQ::DataProvider methods
    
    virtual void setAQNetworkProvider(AQNetworkProvider * aqNetworkProvider);
    
    virtual void setBaseTime (const DateTime & baseTime) throw (BadConfigurationException);
    
    virtual TimeInterval getTimeResolution();
    
    virtual std::pair<const TimeInterval, const TimeInterval> getRange();
    
    virtual std::pair<const TimeInterval, const TimeInterval> getRange(const ForecastHorizon & forecastHorizon);
    
    virtual unsigned int size();
    
    virtual double getNoData();
    
    /**
     * The id parameter is ignored
     */
    virtual double getNoData(const std::string & id);
    
    virtual std::vector<double> getValues(const TimeInterval & beginOffset,
					  const TimeInterval & endOffset, 
					  const std::string & pollutant,
					  const std::string & station);

    virtual std::vector<double> getValues(const std::string & modelName, 
					  const TimeInterval & beginOffset,
					  const TimeInterval & endOffset, 
					  const std::string & pollutant,
					  const std::string & station){
      throw RunTimeException( "AsciiObsProvider not quited for model data, only observations" );
    }
    
    /**
     * The forecastHorizon parameter will be ignored.
     */
    virtual std::vector<double> getValues(const TimeInterval & beginOffset,
					  const TimeInterval & endOffset, 
					  const std::string & pollutant,
					  const std::string & station,
					  const ForecastHorizon & forecastHorizon);

    virtual std::vector<double> getValues(const std::string & modelName, 
					  const TimeInterval & beginOffset,
					  const TimeInterval & endOffset, 
					  const std::string & pollutant,
					  const std::string & station,
					  const ForecastHorizon & forecastHorizon) {
      throw RunTimeException( "AsciiObsProvider not quited for model data, only observations" );
    }
    
    /**
     * The forecastHorizon parameter will be ignored
     */
    virtual std::vector<double> getValues(const std::string & pollutant,
					  const TimeInterval & offset = TimeInterval(0),
					  const ForecastHorizon & forecastHorizon = ForecastHorizon(0) );
    
  private:
    double _noData;
    int    _startCol;
    double _scaleValue;
    TimeInterval _timeResolution;
    DateTime _begin, _end;
    // pollutant -> (station -> data)
    std::map<std::string, std::map<std::string, std::vector<double> > > _buffer;
    std::vector<double> _empty;
    std::string _pattern;
    AQNetworkProvider * _aqNetworkProvider;
    DateTime _baseTime;
    long _beginOffset, _endOffset;
    bool _configured, _baseTimeSet, _clearBufferOnBaseTimeReset;
    
    void _checkFullyConfigured() throw (NotConfiguredException);
    
    void _calculateBeginEnd ();
    
    std::vector<double> & _getValues(const std::string & pollutant,
				     const std::string & station);
    
    void _readFile (const std::string & pollutant, const std::string & station );
    
    std::vector<double> * _getOrInitValues(const std::string & pollutant, const std::string & station);
  };
  
} /* namespace OPAQ */
#endif /* ASCOBSPROVIDER_H_ */
