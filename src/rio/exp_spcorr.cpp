#include <tinyxml.h>

#include <boost/format.hpp>

#include "exp_spcorr.hpp"

namespace rio
{

exp_spcorr::exp_spcorr( const std::string& config_file)
  : correlationmodel()
  , _curr_sel("")
  , _long_a(0.)
  , _long_tau(0.)
  , _short_a(0.)
  , _short_b(0.)
  , _range(0.)
  , _use_short(false)
{
  _name = "exp_spcorr";

  // Get filename with parameters and import as a boost property tree
  std::cout << " Importing spatial correlation model config from : " << config_file << std::endl;
  TiXmlDocument xml;
  if ( ! xml.LoadFile( config_file ) ) throw std::runtime_error( "unable to load/parse xml file " + config_file );
  TiXmlElement *rootEl = xml.FirstChildElement( "correlationmodel" ); 
  if ( ! rootEl ) throw std::runtime_error( "no <correlationmodel> root element found in " + config_file );

  // Look up the class... is this the correct config file ?
  TiXmlElement *classEl = rootEl->FirstChildElement( "class" ); 
  if ( ! classEl ) throw std::runtime_error( "no <class> element found in " + config_file );
  std::string model_class = classEl->GetText();
  boost::trim( model_class );
  if ( model_class.compare( "exp_spcorr" ) )
       throw std::runtime_error( "spatial correlation model config type does not match, expected exp_spcorr" );

  
  // Look up the parameters into boost property tree
  TiXmlElement *paramEl = rootEl->FirstChildElement( "param" );
  if ( ! paramEl ) throw std::runtime_error( "no <param> root element found in " + config_file );
  std::stringstream ss;
  ss << (*paramEl);
  pt::read_xml( ss, _param, pt::xml_parser::trim_whitespace | pt::xml_parser::no_comments );

  // Loop up short_range attribute
  TiXmlElement *rangeEl = rootEl->FirstChildElement( "short_range" );
  if ( rangeEl ) {
    std::string act = rangeEl->Attribute( "active" );
    if ( ! act.compare( "1" )  ||
	 ! act.compare( "on" )  ||
	 ! act.compare( "yes" ) ||
	 ! act.compare( "true" ) ) {
      
      _range = boost::lexical_cast<double>(rangeEl->GetText());
      _use_short = true;

      std::cout << " Using short range of " << _range << " km (note: only parameters, range not used !)" << std::endl; 
    }
  }

  
  // debug:
  /*
    std::stringstream ss;
    pt::write_xml(ss, _param);
    std::cout << ss.str() << std::endl;
    std::cout << "a   = " << _param.get<double>("param.week.h01.a") << std::endl;
    std::cout << "tau = " << _param.get<double>("param.week.h01.tau") << std::endl;
  */
}

exp_spcorr::~exp_spcorr()
{
}

void exp_spcorr::select( const std::string& aggr, boost::posix_time::ptime tstart )
{
  /*
  // Get week/weekend from tstart day
  std::string wk;
  if ( tstart.date().day_of_week() == boost::date_time::Saturday ||
       tstart.date().day_of_week() == boost::date_time::Sunday ) {
    wk = "weekend.";
  } else {
    wk = "week.";
  }
  */
  
  // construct aggregation selection
  std::stringstream ss;
  if ( ! aggr.compare( "1h" ) )
  {
    // in case of 1h aggregation, add the hour of the day, otherwise just use the aggregation
    long hr = tstart.time_of_day().hours()+1;
    ss << boost::format("h%02d") % hr;
  } else ss << aggr;

  // is re-selecting needed ? --> multiple days which are week/weekend
  std::string selection = std::string("param.") + ss.str();

  // std::cout << "requested : " << selection << ", curr = " << _curr_sel << std::endl;
  if ( selection.compare( _curr_sel ) )
  {
    // std::cout << "selection changed, updating parameters..." << std::endl;
    try {

      try {
	_long_a   = _param.get<double>( selection + ".long." + std::string("a") );
	_long_tau = _param.get<double>( selection + ".long." + std::string("tau") );
      } catch ( ... ) {
	_long_a   = _param.get<double>( selection + "." + std::string("a") );
	_long_tau = _param.get<double>( selection + "." + std::string("tau") );
      }

      if ( _use_short )
      {
	_short_a  = _param.get<double>( selection + ".short." + std::string("a") );
	_short_b  = _param.get<double>( selection + ".short." + std::string("b") );
      } 

    } catch ( const std::exception& e ) {
      throw std::runtime_error( "cannot find spcorr parameters for " + selection
				+ ":" + e.what() );
    }

    _curr_sel = selection;
    
  } else {
    // std::cout << "selection is the same, leaving parameters alone..." << std::endl;
  }
    

  return;
}


double exp_spcorr::calc( double x1, double y1, double x2, double y2 )
{
  double rho = 0;
  double r = 1.e-3 * sqrt( (x2-x1)*(x2-x1)+ ( y2-y1)*(y2-y1) );

  // distance < 1 m : return 1 as correlation
  if ( r < 1.e-3 ) return 1.0; 

  // COMPATIBILITY ISSUE
  // TODO: the range is not used as such in the fortran... hmm interesting :)

  double corr_l = _long_a * exp( -r / _long_tau);

  if ( _use_short ) {
    double corr_s = _short_a * r + _short_b;
    rho = std::max( corr_l, corr_s );
  } else {
    rho = corr_l;
  }

  if ( rho > 1. ) return 1.;
  if ( rho < 0. ) return 0.;
  
  return rho;
}
  
}
