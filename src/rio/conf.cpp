#include "conf.hpp"
#include "xmltools.hpp"
#include "strfun.hpp"
#include "parser.hpp"

#include <boost/program_options.hpp>

#include <iostream>
#include <stdexcept>


namespace rio
{

namespace po = boost::program_options;

config::config()
: _domRoot( nullptr )
, _stationConfig( nullptr )
{
  // create the supported formates for date/time parsing
  _formats.push_back( std::locale( std::locale::classic(), new boost::posix_time::time_input_facet("%Y-%m-%d %H:%M:%S") ) );
  _formats.push_back( std::locale( std::locale::classic(), new boost::posix_time::time_input_facet("%Y-%m-%d") ) );
}

  
config::~config()
{
}
  
boost::posix_time::ptime config::_parse_datetime( const char* s )
{

  boost::posix_time::ptime pt;
  for(size_t i=0; i<_formats.size(); ++i)
  {
    std::istringstream is(s);
    is.imbue( _formats[i] );
    is >> pt;
    if ( pt != boost::posix_time::ptime() ) return pt;
  }

  return boost::posix_time::ptime();
}
  
void config::_get_defaults( TiXmlElement *el )
{

  if ( ! el ) throw std::runtime_error( "no defaults in setup xml file..." );

  std::cout << "Reading defaults from setup file..." << std::endl;
  try {
    _base = rio::xml::getText( el, "base_path" );
  } catch ( ... ) {
    _base = ".";
  }
  if ( _base.empty() ) _base = ".";

  if (_cnf.empty()) {
    _cnf  = rio::xml::getText( el, "configuration" );
  }

  if (_pol.empty()) {
    _pol  = rio::xml::getText( el, "pollutant" );
  }

  if (_aggr.empty()) {
    _aggr = rio::xml::getText( el, "aggregation" );
  }

  if (_ipol.empty()) {
    _ipol = rio::xml::getText( el, "mapper" );
  }

  if (_grd.empty()) {
    _grd  = rio::xml::getText( el, "grid" );
  }

  if (_out.empty()) {
    _out  = { rio::xml::getText( el, "output" ) };
  }
}


// process the XML with the selected configuration elemetns
void config::_get_runconfig()
{

  if ( ! _domRoot ) throw std::runtime_error( "_configure() :: no <rio> root element in setup file" );

  // find the requested configuration
  TiXmlElement *cnfEl;
  try{ 
    cnfEl = rio::xml::getElementByAttribute( _domRoot, "configuration", "name", _cnf );
  } catch( ... ) {
    throw std::runtime_error( "Cannot find requested configuration " + _cnf + " in XML setup file" );
  }

  // some general, configuration specific configuration
  _author = rio::xml::getChildValue( cnfEl, "author" );
  _email  = rio::xml::getChildValue( cnfEl, "email" );
  _desc   = rio::xml::getChildValue( cnfEl, "description" );

  try { _missing_value = boost::lexical_cast<double>( rio::xml::getChildValue( cnfEl, "missing_value" ) );
  } catch ( ... )  { _missing_value = -9999.; }

  // find a *generic* station and grid tag, can be overwritten later on
  _stationConfig = cnfEl->FirstChildElement( "stations" );
  _gridConfig    = cnfEl->FirstChildElement( "grids" );

  // find the observations
  _obsConfig = cnfEl->FirstChildElement( "observations" );
  if ( ! _obsConfig ) throw std::runtime_error( "cannot find <observations> element in configuration " + _cnf );  
   
  // get the model configuration
  TiXmlElement *polEl = NULL;
  try {
    std::vector<std::string> attNames = { "name", "aggr" };
    std::vector<std::string> attVals  = { _pol, _aggr };
    polEl = rio::xml::getElementByAttributesList( cnfEl, "pollutant", attNames, attVals );

  } catch ( ... ) {
    throw std::runtime_error( "cannot find <pollutant> element with name=" + _pol +
			      ", and aggr=" + _aggr + " in configuration " + _cnf );
  }

  // some additional pollutant specific configuration
  try { _detection_limit = boost::lexical_cast<double>( rio::xml::getChildValue( polEl, "detection_limit" ) );
  } catch ( ... )  { _detection_limit = 1.0; }
  
  // now check the model
  try {
    _modelConfig = rio::xml::getElementByAttribute( polEl, "mapper", "name", _ipol );
  } catch ( ... ) {
    throw std::runtime_error( "cannot find <mapper> element with name=" + _ipol + " for " + _pol + ", " + _aggr );
  }

  // Overwrite generic station and grid with the one inside the model, check later
  TiXmlElement *p = NULL;
  if ( p = _modelConfig->FirstChildElement( "stations" ) ) {
    std::cout << "Overwriting <stations> with " + _ipol + " model specific configuration" << std::endl;
    _stationConfig = p;
  }
  p = NULL;
  if ( p = _modelConfig->FirstChildElement( "grids" ) ) {
    std::cout << "Overwriting <grids> with " + _ipol + " model specific configuration" << std::endl;
    _gridConfig = p;
  }
  
  // get some info from the model
  _ipol_class = _modelConfig->Attribute( "class" );

  // do we have everything ?
  if ( ! _stationConfig ) throw std::runtime_error( "cannot find <stations> element in configuration " + _cnf );  
  if ( ! _gridConfig ) throw std::runtime_error( "cannot find <grids> element in configuration " + _cnf );  
}

void config::parse_setup_file(const std::string& setup_file)
{
  // try to import the setup file
  std::cout << "Using deployment in : " << setup_file << std::endl; 
  if ( ! _dom.LoadFile( setup_file ) ) throw std::runtime_error( "unable to load/parse xml setupfile : " + setup_file );

  _domRoot = _dom.FirstChildElement( "rio" );
  if ( ! _domRoot ) throw std::runtime_error( "no <rio> root element in setup file" );

  // set some defaults from DOM
  _get_defaults( _domRoot->FirstChildElement( "defaults" ) );

  // get output configuration    
  _outputConfig = _domRoot->FirstChildElement( "output" );
  if ( ! _outputConfig ) throw std::runtime_error( "cannot find <output> element in setup file" );

  // read the xml file and parse
  _get_runconfig();

  // update rio parser
  _update_parser();

}
  
void config::_update_parser( void  ) {
 
 // set the default wildcards, the time will be set in the loop
  rio::parser::get()->add_pattern( "%base%", _base );
  rio::parser::get()->add_pattern( "%cnf%",  _cnf );
  rio::parser::get()->add_pattern( "%pol%",  _pol );
  rio::parser::get()->add_pattern( "%aggr%", _aggr );
  rio::parser::get()->add_pattern( "%ipol%", _ipol );
  rio::parser::get()->add_pattern( "%grid%", _grd );

  // add start and end times for the requested run
  rio::parser::get()->add_pattern( "%start_time%", boost::posix_time::to_iso_string( _tstart ) );
  rio::parser::get()->add_pattern( "%end_time%",  boost::posix_time::to_iso_string( _tstop ) );

  return;
}

void config::parse_command_line( int argc, char *argv[] )
{  
  po::options_description optionsDesc("Available options");
  optionsDesc.add_options()
    ("base,b", po::value<std::string>(&_base)->default_value(".")->value_name("<path>"), "base path for this run")
    ("cnf,c", po::value<std::string>(&_cnf)->value_name("<name>"), "RIO configuration")
    ("pol,p", po::value<std::string>(&_pol)->value_name("<name>"), "pollutant")
    ("aggr,a", po::value<std::string>(&_aggr)->value_name("<1h,da,m1,m8,wk>"), "aggregation")
    ("ipol,i", po::value<std::string>(&_ipol)->value_name("<name>"), "interpolation model")
    ("grid,g", po::value<std::string>(&_grd)->value_name("<name>"), "grid name")
    ("output,o", po::value<std::string>()->default_value("")->value_name("<type1>[,type2,...]"), "output option, comma separated [txt,hdf5,...]")
    ("help,h", "show this message.")
    ("debug,d", "switch on debugging mode")
  ;

  po::options_description pidesc("Position independant options");
  pidesc.add_options()
    ("tstart", po::value<std::string>()->default_value(""), "start time")
    ("tstop", po::value<std::string>()->default_value(""), "stop time")
  ;

  po::options_description all("All options");
  all.add(optionsDesc).add(pidesc);

  po::positional_options_description pd;
  pd.add("tstart", 1).add("tstop", 1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(all).positional(pd).run(), vm);
  po::notify(vm);

  if (vm.count("help") > 0)
  {
    //std::cout << desc << std::endl;
    std::cout << "Usage:\n"
              << "  rio [options] tstart [tstop]\n\n"
              << optionsDesc
              << "Supported datetime format : yyyy-mm-dd[ HH:MM:SS]" << std::endl;
    std::exit(EXIT_SUCCESS);
  }

  if (vm.count("output") > 0)
  {
    // parse the list of options
	  boost::split(_out, vm["output"].as<std::string>(), boost::is_any_of(",;"));

    // handle empty ones if the user should put a trailing comma or semi colon after the list
	  auto it = std::remove_if(_out.begin(), _out.end(), [](const std::string& s) { return s.empty(); });
	  _out.erase(it, _out.end());
  }
  
  _debug = vm.count("debug") > 0;
  
  // parsing date/time
  if (vm.count("tstart") == 1) {
    _tstart = _parse_datetime(vm["tstart"].as<std::string>().c_str());
  } else {
    throw std::runtime_error("*** error parsing command line options, see --help !");
  }

  if (vm.count("tstop") == 1) {
    _tstop = _parse_datetime(vm["tstop"].as<std::string>().c_str());
    if ( _tstop.is_not_a_date_time() )
    {
      // fix as the argument is there as "" with the boost argument parser.. grmmbl
      std::cout << "Requested single date : " << _tstart << std::endl;
      _tstop  = _tstart;
    } else {
      std::cout << "Requested date range : " << _tstart << " to " << _tstop << std::endl;
    }
  } else {
    std::cout << "Requested single date : " << _tstart << std::endl;
    _tstop  = _tstart;
  }

  // set timestep
  if ( ! _aggr.compare( "1h" )  ) _tstep = boost::posix_time::hours(1);
  else _tstep = boost::posix_time::hours(24);

}

std::ostream& operator<<(std::ostream& output, const config& c) {

  output << "[Configuration]" << std::endl;
  output << " Debugging mode     : " << ( c._debug ? "on" : "off" ) << std::endl; 
  output << " Base path          : " << c._base << std::endl;
  output << " Configuration      : " << c._cnf << std::endl;
  output << " Pollutant          : " << c._pol << std::endl;
  output << " Aggregation        : " << c._aggr<< std::endl;
  output << " Interpolator       : " << c._ipol << std::endl;
  output << " Grid               : " << c._grd << std::endl;
  output << " Output mode        :";
  for ( auto s : c._out ) output << " " << s;
  output << std::endl;

  output << "[Date range]" << std::endl;
  output << " Start time : " << boost::posix_time::to_simple_string( c._tstart ) << std::endl;
  output << " Stop time  : " << boost::posix_time::to_simple_string( c._tstop ) << std::endl;
  return output;  // for multiple << operators.
}


} //namespace
