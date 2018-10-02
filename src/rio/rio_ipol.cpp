#include "rio_ipol.hpp"

#include "parser.hpp"
#include "exp_spcorr.hpp"
#include "polytrend.hpp"


namespace rio
{

rio_ipol::rio_ipol( const config& cf,
		    const std::shared_ptr<rio::network>& net )
  : mapper( cf, net )
  , _logtrans(false)
{
  _name = "RIO";

  // 0. -- Process generic options
  try {
    std::string slt = _opts.get<std::string>( "options.logtrans" );
    boost::trim( slt );
    if ( ! slt.compare( "on" ) ||  ! slt.compare( "true" ) || ! slt.compare( "enable" ) ) _logtrans = true;
  } catch ( ... ) {
    _logtrans = false;
  }

  // 1. -- Configure the spatial correlation model
  TiXmlElement *spEl = cf.modelConfig()->FirstChildElement( "correlationmodel" );
  if ( ! spEl ) throw std::runtime_error( "no correlation model specified in rio_ipol mapper" );

  std::string sp_config = spEl->GetText();
  rio::parser::get()->process( sp_config );

  
  std::string sp_type = spEl->Attribute( "class" );
  if ( ! sp_type.compare( "exp_spcorr" ) )
  {
    std::cout << " Set correlation model : exp_spcorr" << std::endl;
    _spcorr = std::make_unique<rio::exp_spcorr>( sp_config );

  } else {
    throw std::runtime_error( "requested spatial correlation model \"" + sp_type + "\" in rio_ipol is not supported..." );
  }


  // 2. -- Configure the trend model
  TiXmlElement *trEl = cf.modelConfig()->FirstChildElement( "trendmodel" );
  if ( ! trEl ) throw std::runtime_error( "no trend model specified in rio_ipol mapper" );

  std::string tr_config = trEl->GetText();
  rio::parser::get()->process( tr_config );
  
  std::string tr_type = trEl->Attribute( "class" );
  if ( ! tr_type.compare( "polytrend" ) )
  {
    std::cout << " Set trend model : polytrend" << std::endl;
    _trend = std::make_unique<rio::polytrend>( tr_config );

  } else {
    throw std::runtime_error( "requested trendmodel \"" + tr_type + "\" in rio_ipol is not supported..." );
  }
  
}

rio_ipol::~rio_ipol( )
{
}


void rio_ipol::run( Eigen::VectorXd& values,
                    Eigen::VectorXd& uncert,
		                boost::posix_time::ptime tstart,
		                const std::map<std::string, double>& obs,
		                const std::shared_ptr<rio::grid>& g ) const
{

  // get the stations x and y values and observations
  Eigen::VectorXd xs( obs.size() );
  Eigen::VectorXd ys( obs.size() );
  Eigen::VectorXd  V( obs.size() );

  size_t i = 0;

  // prepare correlation & trend model
  _spcorr->select( _aggr, tstart );
  _trend->select( _aggr, tstart );


  // detrend station values
  for ( const auto& it : obs ) {

    std::shared_ptr<rio::station const> st = _net->get( it.first );
    if (  st == nullptr ) throw std::runtime_error( "internal error, station not found" );
    
    xs[i] = st->x();
    ys[i] = st->y();
    V[i]  = it.second;

    // detrend --> add station name to look up certain parameters for the station...
    _trend->detrend( V[i], it.first, st->proxy() );

    // logtrans ?
    if ( _logtrans ) V[i] = log(1. + V[i] );
    
    i++;
  }

  // calculate variance of the sample we interpolate, needed for the error
  double xx_var = (V.array() - V.mean()).square().sum()/(V.size()-1);

  // allocate station covariance matrix
  Eigen::MatrixXd C = Eigen::MatrixXd::Ones( xs.size()+1, xs.size()+1 );

  for ( unsigned int i = 0; i < xs.size(); i++ ) {
    for ( unsigned int j = 0; j <= i; j++ ) {
      
      double rho = _spcorr->calc( xs[i], ys[i], xs[j], ys[j] );      

      C(i,j) = rho;
      if ( i != j ) C(j,i) = rho;
      
    }
  }
  C( xs.size(), xs.size() ) = 0.;
  
  // allocate matrix of covariance between stations and gridcell : [ D1; D2; D3 ... ] for all grids
  // note: gridcells in the rows
  Eigen::MatrixXd D = Eigen::MatrixXd::Ones( g->size(), xs.size()+1 );
  
  for ( unsigned int i = 0; i < g->size(); i++ ) {
    for ( unsigned int j = 0; j < xs.size(); j++ ) {
      
      D(i,j) = _spcorr->calc( xs[j], ys[j], g->cells()[i].cx(), g->cells()[i].cy() );

    }
  }

  // solve the whole grid using some heavy vectorization...
  Eigen::MatrixXd w = C.ldlt().solve(D.transpose()).transpose();
  values = w.block(0,0,g->size(),xs.size())*V;  
  uncert = sqrt( xx_var * ( Eigen::VectorXd::Ones(g->size(),1).array() - 
                           ( w.array() * D.array() ).rowwise().sum().array() ) ); // ng x ns+1

  if ( _logtrans )
  {
    uncert.array() = sqrt(exp(2.*values.array())*uncert.array().square());
    values.array() = exp(values.array())-1.0;
  }
  
  // add trend
  for ( unsigned int i = 0; i < g->size(); i++ ) {
    _trend->addtrend( values(i), uncert(i), g->cells()[i].proxy() );
  
      if ( std::isnan( values(i) )  ||
	       std::isinf( values(i) ) ) {
      values(i) = _missing_value;
      uncert(i) = _missing_value;
    }

    if ( values(i) < _detection_limit ) values(i) = _detection_limit;
  }



  return;
}

}
