/*
 *
 * AsciiForecastWriter.h
 *
 */
#ifndef AsciiForecastWriter_H_
#define AsciiForecastWriter_H_

#include <string>
#include <vector>

#include "opaq.h"

namespace OPAQ {

class AsciiForecastWriter : public OPAQ::ForecastOutputWriter {

 public:
  AsciiForecastWriter();
  virtual ~AsciiForecastWriter();

  static const std::string BASETIME_PLACEHOLDER;
  static const std::string POLLUTANT_PLACEHOLDER;
  static const std::string AGGREGATION_PLACEHOLDER;

  // OPAQ::Component methods
  // throws (BadConfigurationException)
  virtual void configure(TiXmlElement * configuration, IEngine& engine);


  // OPAQ::ForecastOutputWriter methods
  virtual void write( OPAQ::Pollutant *pol, OPAQ::Aggregation::Type aggr, const OPAQ::DateTime &baseTime );

private:
  Logger                   _logger;
  std::string              _filename;
  std::vector<std::string> _models;       //! a list of models to output
  std::string              _title;
  std::string              _header;
  bool                     _enable_fields;
  char                     _sepchar;      //! fields separation character, default is tab
  bool                     _fctime_full;  //! output full fctime
  bool                     _full_output;  //! output all the stations, if false will only ouptut stations that measure the pollutant
};



} // namespace

#endif /* #ifndef AsciiForecastWriter_H_ */