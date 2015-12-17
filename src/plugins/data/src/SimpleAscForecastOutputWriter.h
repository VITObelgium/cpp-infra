/*
 *
 * SimpleAscForecastOutputWriter.h
 *
 */
#ifndef SIMPLEASCFORECASTOUTPUTWRITER_H_
#define SIMPLEASCFORECASTOUTPUTWRITER_H_

#include <opaq/pfw.h>
#include <opaq/common.h>

namespace OPAQ {

class SimpleAscForecastOutputWriter : public OPAQ::ForecastOutputWriter {

 public:
  SimpleAscForecastOutputWriter();
  virtual ~SimpleAscForecastOutputWriter();

  
  LOGGER_DEC();

  static const std::string BASETIME_PLACEHOLDER;
  static const std::string POLLUTANT_PLACEHOLDER;

  // OPAQ::Component methods
  virtual void configure(TiXmlElement * configuration)
    throw (BadConfigurationException);


  // OPAQ::ForecastOutputWriter methods
  virtual void write( OPAQ::Pollutant *pol, const OPAQ::DateTime &baseTime );

private:
  std::string _filename;

};



} // namespace

#endif /* #ifndef SIMPLEASCFORECASTOUTPUTWRITER_H_ */ 
