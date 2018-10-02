#include "dbqfile.hpp"
#include "strfun.hpp"

#include <stdexcept>
#include <iostream>
#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost::gregorian;
using namespace boost::posix_time;

namespace rio
{


dbqfile::dbqfile( TiXmlElement *cnf )
{
  /*
     <observations>
      <dbqfile type="RIO" scale="1.0">%base%/data/%pol%_data_rio.txt</dbqfile>
    </observations>
  */
  if ( ! cnf ) throw std::runtime_error( "Invalid TiXmlElement pointer in dbqfile()..." );

  TiXmlElement *fEl = cnf->FirstChildElement( "dbqfile" );
  if ( ! fEl ) throw std::runtime_error( "Cannot find <dbqfile> element in observations..." );

  // get the scale attribute
  if ( fEl->QueryDoubleAttribute( "scale", &_scale ) != TIXML_SUCCESS ) _scale = 1.0;

  // get the type attribute
  std::string type;
  if ( fEl->QueryStringAttribute( "type", &type ) != TIXML_SUCCESS )
  {
    std::cout << "+++ warning: assuming dbqfile type RIO, no type attribute given in <dbqfile>..." << std::endl;
    type = "RIO";
  }

  // get the filename
  std::string filename = fEl->GetText();

  // run through the parser
  rio::parser::get()->process( filename );

  // load the file
  if ( boost::iequals( type, "RIO" ) )
  {
    load_riofile( filename );
  } else {
    throw std::runtime_error( "File type " + type + " is not supported by dbqfile" );
  }

}

dbqfile::dbqfile( std::string filename, std::string type, double scale )
  : _scale( scale )
{
  
  if ( boost::iequals( type, "RIO" ) )
  {
    load_riofile( filename );
  } else {
    throw std::runtime_error( "File type " + type + " is not supported by dbqfile" );
  }
  
}

  
void dbqfile::load_riofile( std::string filename )
{
  std::cout << " Importing " << filename << std::endl;
  // parse whole file & store in database...

  FILE *fp = fopen( filename.c_str(), "r" );
  if ( ! fp ) throw std::runtime_error( "Unable to open file: " + filename );

  char  line[rio::strfun::LINESIZE];
  char *ptok;

  while ( fgets( line, sizeof(line), fp ) )
  {
    if (rio::strfun::trim(line)[0] == '#' ) continue;
    if ( ! strlen( rio::strfun::trim(line) ) ) continue;

    if ( !( ptok = strtok( line, rio::strfun::SEPCHAR ) ) ) continue; 
    std::string st_name = ptok;
    if ( !( ptok = strtok( NULL, rio::strfun::SEPCHAR ) ) ) continue; 
    std::string date_str = ptok;
    int year = atoi( date_str.substr(0,4).c_str() );
    int mon  = atoi( date_str.substr(4,2).c_str() );
    int day  = atoi( date_str.substr(6,2).c_str() );
    
    // construct the date& time periods
    boost::posix_time::ptime t1( boost::gregorian::date( year, mon, day ) );
    boost::posix_time::time_period dt( t1, hours(24) );    

    // m1
    if ( !( ptok = strtok( NULL, rio::strfun::SEPCHAR ) ) ) continue; 
    double m1 = atof(ptok);
    _dbm1[st_name].insert( dt, _scale*m1 );
    
    // m8
    if ( !( ptok = strtok( NULL, rio::strfun::SEPCHAR ) ) ) continue; 
    double m8 = atof(ptok);
    _dbm8[st_name].insert( dt, _scale*m8 );
    
    // da
    if ( !( ptok = strtok( NULL, rio::strfun::SEPCHAR ) ) ) continue; 
    double da = atof(ptok);
    _dbda[st_name].insert( dt, _scale*da );
    
    // 24 hourly values
    boost::posix_time::time_period dt1h( t1, hours(1) );    
    while( ptok = strtok( NULL, rio::strfun::SEPCHAR ) )
    {
      double xx1h = atof(ptok);
      _db1h[st_name].insert( dt1h, _scale*xx1h );
      dt1h.shift(hours(1));
    }
    
  }

  fclose(fp);

  // write some info about the timeseries... 
  //write_summary();
  return;
}

dbqfile::~dbqfile()
{
}
  
void dbqfile::get( std::map<std::string, double>& data,
		   boost::posix_time::ptime tstart, std::string pol, std::string agg )
{
  // clear  map
  data.clear();

  // get pointer to correct database archive, depending on aggregation
  std::map<std::string, rio::timeseries> *_db;
  if ( ! agg.compare( "m1" ) ) {
    _db = &_dbm1;
  } else if ( ! agg.compare( "m8" ) ) {
    _db = &_dbm8;
  } else if ( ! agg.compare( "da" ) ) {
    _db = &_dbda;
  } else if ( ! agg.compare( "1h" ) ) {
    _db = &_db1h;
  } else {
    throw std::runtime_error( "no such aggregation time : " + agg );
  }

  bool missing = false;
  for ( const auto& it :  *_db ) {
    double x = it.second.get( tstart, missing );
    if ( ! missing ) {

      // if we have set the network, only return data for stations in the network, otherwise return all
      if ( _net ) {
	if ( ! _net->get( it.first ) ) continue;
      }  

      data[it.first] = x; // otherwise don't insert in the map
    }
  }
  
  return;
}


void dbqfile::write_summary( void )
{

  std::cout << "[List of stations]" << std::endl;
  for ( const auto& it :  _dbm1 ) {
    std::cout << it.first << std::endl;
    std::cout << " - da range : "
	      << _dbda[it.first].first_time() << " - "
	      << _dbda[it.first].last_time()
	      << ", step : " << _dbda[it.first].interval()
	      << ", size : " << _dbda[it.first].size() << std::endl;
    std::cout << " - m1 range : "
	      << _dbm1[it.first].first_time() << " - "
	      << _dbm1[it.first].last_time()
	      << ", step : " << _dbm1[it.first].interval()
	      << ", size : " << _dbm1[it.first].size() << std::endl;
    std::cout << " - m8 range : "
	      << _dbm8[it.first].first_time() << " - "
	      << _dbm8[it.first].last_time()
	      << ", step : " << _dbm8[it.first].interval()
	      << ", size : " << _dbm8[it.first].size() << std::endl;
    std::cout << " - 1h range : "
	      << _db1h[it.first].first_time() << " - "
	      << _db1h[it.first].last_time()
	      << ", step : " << _db1h[it.first].interval()
	      << ", size : " << _db1h[it.first].size() << std::endl;
      
  }

  return;
}
  
}
