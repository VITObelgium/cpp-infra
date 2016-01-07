/*
 *
 * AsciiForecastWriter.h
 *
 */
#ifndef AsciiForecastWriter_H_
#define AsciiForecastWriter_H_

#include <string>
#include <vector>

#include <opaq.h>

namespace OPAQ {

class AsciiForecastWriter : public OPAQ::ForecastOutputWriter {

 public:
  AsciiForecastWriter();
  virtual ~AsciiForecastWriter();

  
  LOGGER_DEC();

  static const std::string BASETIME_PLACEHOLDER;
  static const std::string POLLUTANT_PLACEHOLDER;
  static const std::string AGGREGATION_PLACEHOLDER;

  // OPAQ::Component methods
  virtual void configure(TiXmlElement * configuration)
    throw (BadConfigurationException);


  // OPAQ::ForecastOutputWriter methods
  virtual void write( OPAQ::Pollutant *pol, OPAQ::Aggregation::Type aggr, const OPAQ::DateTime &baseTime );

private:
  std::string              _filename;
  std::vector<std::string> _models;   //! a list of models to output
  std::string              _title;
  std::string              _header;
  bool                     _enable_fields;
};



} // namespace

#endif /* #ifndef AsciiForecastWriter_H_ */