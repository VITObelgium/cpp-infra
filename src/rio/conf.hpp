#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <tinyxml.h>

namespace rio
{

class config
{
public:
  config();
  virtual ~config();
  
  void parse_setup_file(const std::string& setup_file);
  void parse_command_line( int argc, char *argv[] );

  bool debug( void ) const { return _debug; }
  
  friend std::ostream& operator<<(std::ostream& output, const config& c);

  std::string base_path( void ) const { return _base; }
  std::string pol( void ) const { return _pol; }
  std::string aggr( void ) const { return _aggr; }
  std::string grid( void ) const { return _grd; }
  std::string ipol( void ) const { return _ipol; }
  std::string configuration(void) const { return _cnf; };

  boost::posix_time::ptime start_time( void ) const { return _tstart; }
  boost::posix_time::ptime stop_time( void ) const { return _tstop; }

  boost::posix_time::time_duration tstep( void ) const { return _tstep; }

  // probably a bad idea this, need to use smart pointers...
  TiXmlElement* stationConfig() const { return _stationConfig; }
  TiXmlElement* gridConfig() const { return _gridConfig; }
  TiXmlElement* obsConfig() const { return _obsConfig; }
  TiXmlElement* modelConfig() const { return _modelConfig; }
  
  TiXmlElement* outputConfig() const { return _outputConfig; }
  const std::vector<std::string>& req_output () const { return _out; }
  

  // mapping model selection
  const std::string& ipol_class() const { return _ipol_class; }
  double             detection_limit() const { return _detection_limit; }
  double             missing_value() const { return _missing_value; }
  const std::string& author() const { return _author; }
  const std::string& email() const { return _email; }
  const std::string& desc() const { return _desc; }


private:
  bool _debug;

  std::string _base; //! basepath  
  std::string _cnf;  //! configuration

  std::string _pol;  //! running for pollutant
  std::string _aggr; //! running for this aggregation time
  std::string _grd;  //! running for this grid
  std::string _ipol; //! mapper

  std::vector<std::string> _out;  //! output options
  
  boost::posix_time::ptime _tstart;
  boost::posix_time::ptime _tstop;
  boost::posix_time::time_duration _tstep;
  
  TiXmlDocument _dom;      //! setup xml document model
  TiXmlElement* _domRoot;  //! root element in DOM
  
  TiXmlElement* _stationConfig; //! pointer to the station config element

  // replace this... 
  TiXmlElement* _gridConfig;    //! ditto for the grid

  TiXmlElement* _obsConfig;     //! ditto for the observations
  TiXmlElement* _outputConfig;  //! and for the outpu
  TiXmlElement* _modelConfig;   //! model config
  
  std::string   _ipol_class;    
  double        _detection_limit;
  double        _missing_value; 
  std::string   _author;
  std::string   _email;
  std::string   _desc;

private:
  // some helper routines
  void _get_defaults( TiXmlElement *el );
  void _get_runconfig( void );
  void _update_parser( void );
  
  std::vector<std::locale> _formats;
  boost::posix_time::ptime _parse_datetime( const char* s );
};

}
