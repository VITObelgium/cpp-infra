#include "asciiwriter.hpp"
#include "parser.hpp"

namespace rio
{

asciiwriter::asciiwriter( TiXmlElement *cnf )
  : outputhandler(cnf)
  , _pattern("rio_%timestamp%.txt")
  , _fs( '\t' )
  , _pr( 3 )
{
  try {
    _pattern   =  _xml.get<std::string>( "handler.location" );
  } catch ( ... ) {
    throw std::runtime_error( "invalid configuration in asciiwriter XML config, <location> required" );
  }  

  try {
    std::string fs = _xml.get<std::string>( "handler.field_separator" );
    if ( fs == "tab" ) {
      _fs = '\t';
    } else if ( fs.empty() || ( fs == "space" ) ) {
      _fs = ' '; // space when empty string is given
    } else {
      _fs = fs[0]; // only takes first character
    }    
    _pr = _xml.get<int>( "handler.precision" );
  } catch ( ... ) {    
    // using default...
  }

  // probably much better ways of handling this...
  switch( _fs ) 
  { 
    case '\t': // deal with the tab character properly
      sprintf( _fmt, "%%zd\\t%%.0f\\t%%.0f\\t%%.%df\\t%%.%df\\n", _pr, _pr );
      break;
    default:
      sprintf( _fmt, "%%zd%c%%.0f%c%%.0f%c%%.%df%c%%.%df\\n", _fs, _fs, _fs, _pr, _fs, _pr );
      break;
  }
  
  std::cout << "format string : " << _fmt << std::endl;

}

asciiwriter::~asciiwriter()
{
}

void asciiwriter::init( const rio::config& cnf,
			const std::shared_ptr<rio::network> net,
			const std::shared_ptr<rio::grid> grid )
{

  // do some parsing of cnf.getOutput();

  _net     = net;
  _grid    = grid;
  
  return;
}

void asciiwriter::write( const boost::posix_time::ptime& curr_time,
                         const std::map<std::string, double>& obs, 
                         const Eigen::VectorXd& values,
                         const Eigen::VectorXd& uncert )
{
  // dump the map
  std::string ofname = _pattern;
  rio::parser::get()->process( ofname );
  
  FILE *fp = fopen( ofname.c_str(), "wt" );
  fprintf( fp, "FID%cX%cY%cC%cERR\n", _fs, _fs, _fs, _fs );
  for ( size_t i = 0; i < _grid->size(); i++ ) {
    //fprintf( fp, "%zd\t%.0f\t%.0f\t%.3f\t%.3f\n", _grid->cells()[i].id(), _grid->cells()[i].cx(), _grid->cells()[i].cy(), values[i], uncert[i] );
    fprintf( fp, _fmt, 
      _grid->cells()[i].id(), _grid->cells()[i].cx(), _grid->cells()[i].cy(), values[i], uncert[i] );
  }
  fclose( fp );

  
  return;
}

void asciiwriter::close( void )
{
  std::cout << "Closing asciiwriter..." << std::endl;
  return;
}
  
}
