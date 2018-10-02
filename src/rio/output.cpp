#include "output.hpp"

#include "asciiwriter.hpp"
#include "ircelwriter.hpp"
#include "h5writer.hpp"
#include "apswriter.hpp"
#include "jsonwriter.hpp"

#include "xmltools.hpp"

#if defined _MSC_VER
#include <direct.h>
#elif defined __GNUC__
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace rio
{

output::output( TiXmlElement* el, std::vector<std::string> req_outputs  )
{

  if ( ! el ) throw std::runtime_error( "Error in TiXmlElement in outputhandler::select_outputs" );

  for ( const auto& req : req_outputs )
  {

    // lookup the element
    TiXmlElement *outEl;
    try {
      outEl = rio::xml::getElementByAttribute( el, "handler", "name", req );
    } catch ( ... ) {
      throw std::runtime_error( "Requested output type " + req + " not available" );
    }

    std::string req_class;
    try {
      req_class = outEl->Attribute( "class" );
    } catch ( ... ) {
      throw std::runtime_error( "output handler config for " + req + " does not contain class attribute" );
    }

    // check the class
    if ( boost::equals( req_class, "asciiwriter" ) ) {
      _list.push_back( std::make_unique<rio::asciiwriter>( outEl ) );
      std::cout << " Added asciiwriter to output list" << std::endl;

      } else if ( boost::equals( req_class, "ircelwriter" ) ) {
      _list.push_back( std::make_unique<rio::ircelwriter>( outEl ) );
      std::cout << " Added ircelwriter to output list" << std::endl;

    } else if ( boost::equals( req_class, "h5writer" ) ) {
      _list.push_back( std::make_unique<rio::h5writer>( outEl ) );
      std::cout << " Added h5writer to output list" << std::endl;

    } else if ( boost::equals( req_class, "apswriter" ) ) {
      _list.push_back( std::make_unique<rio::apswriter>( outEl ) );
      std::cout << " Added apswriter to output list" << std::endl;

    } else if ( boost::equals( req_class, "jsonwriter" ) ) {
      _list.push_back( std::make_unique<rio::jsonwriter>( outEl ) );
      std::cout << " Added jsonwriter to output list" << std::endl;

    } else {
      throw std::runtime_error( "Invalid output class requested : " + req_class );
    }
  }

  return;
}


output::~output()
{
}


void output::init( const rio::config& cnf,
		   const std::shared_ptr<rio::network> net,
		   const std::shared_ptr<rio::grid> grid )
{
  for ( const auto& w : _list ) w->init( cnf, net, grid );
  return;
}

void output::write( const boost::posix_time::ptime& curr_time,
                    const std::map<std::string, double>& obs,
                    const Eigen::VectorXd& values,
                    const Eigen::VectorXd& uncert )
{
  // make sure output directory exists
  #if defined _MSC_VER
    _mkdir("./output");
  #elif defined __MINGW32__
    mkdir("./output");
  #elif defined __GNUC__
     mkdir("./output", 0777);
  #endif

  for ( const auto& w : _list ) w->write( curr_time, obs, values, uncert );
  return;
}


void output::close( void )
{
  for ( const auto& w : _list ) w->close();
  return;
}

}
