#include <tinyxml.h>

#include "krige_ipol.hpp"
#include "parser.hpp"
#include "exp_spcorr.hpp"

namespace rio
{

krige_ipol::krige_ipol( const config& cf,
			const std::shared_ptr<rio::network>& net )
  : mapper( cf, net )
  , _logtrans(false)
{
  _name = "KRIGING";

  // Configure the spatial correlation model
  TiXmlElement *spEl = cf.modelConfig()->FirstChildElement( "correlationmodel" );
  if ( ! spEl ) throw std::runtime_error( "no correlation model specified in krige_ipol mapper" );

  try {
    std::string slt = _opts.get<std::string>( "options.logtrans" );
    boost::trim( slt );
    if ( ! slt.compare( "on" ) ||  ! slt.compare( "true" ) || ! slt.compare( "enable" ) ) _logtrans = true;
  } catch ( ... ) {
    _logtrans = false;
  }

  std::string sp_config = spEl->GetText();
  rio::parser::get()->process( sp_config );

  std::string sp_type = spEl->Attribute( "class" );
  if ( ! sp_type.compare( "exp_spcorr" ) )
  {
    std::cout << " Set correlation model : exp_spcorr" << std::endl;
    _spcorr = std::make_shared<rio::exp_spcorr>( sp_config );

  } else {

    throw std::runtime_error( "requested spatial correlation model \"" + sp_type + "\" in krige_ipol is not supported..." );
  }
  
}

krige_ipol::~krige_ipol( )
{
}


void krige_ipol::run( Eigen::VectorXd& values,
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
  for ( const auto& it : obs ) {

    std::shared_ptr<rio::station const> st = _net->get( it.first );
    if (  st == nullptr ) throw std::runtime_error( "internal error, station not found" );
    
    xs[i] = st->x();
    ys[i] = st->y();
    V[i]  = it.second;

    // logtrans ?
    if ( _logtrans ) V[i] = log(1. + V[i] );

    i++;
  }

  // calculate variance of the sample we interpolate, needed for the error
  double xx_var = (V.array() - V.mean()).square().sum()/(V.size()-1);

  // prepare the spatial correlation model, e.g. get the right parameters for the time of the day
  _spcorr->select( _aggr, tstart );
  
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

  // compute LU decomposition o we can reuse it in the uncertainty
  // solve the whole grid using some heavy vectorization :-)
  
  // --> did some performance checks here
  
  // 1. calling ldlt on both occasions
  //values = (C.ldlt().solve(D.transpose()).transpose()).block(0,0,g->size(),xs.size())*V;  
  //uncert = sqrt( xx_var * ( Eigen::VectorXd::Ones(g->size(),1).array() - 
  //                         ( ( C.ldlt().solve(D.transpose()).transpose()).array() * D.array() ).rowwise().sum().array() ) ); // ng x ns+1
  
  // 2. precompute LDLT
  /*
  Eigen::LDLT<Eigen::MatrixXd> ldlt(C);
  values = (ldlt.solve(D.transpose()).transpose()).block(0,0,g->size(),xs.size())*V;  
  // now attempt the same for the uncertainty
  // colum, wise dot product is element wise
  // w : ng x ns + 1 ,w = C.ldlt().solve(D.transpose()).transpose()
  // D : ng x ns + 1
  uncert = sqrt( xx_var * ( Eigen::VectorXd::Ones(g->size(),1).array() - 
                           ( ( ldlt.solve(D.transpose()).transpose()).array() * D.array() ).rowwise().sum().array() ) ); // ng x ns+1
 */

  // 3. store intermediate result (w for each gridcell) has best performance, but a bit more memory intensive..
  Eigen::MatrixXd w = C.ldlt().solve(D.transpose()).transpose();
  values = w.block(0,0,g->size(),xs.size())*V;  
  uncert = sqrt( xx_var * ( Eigen::VectorXd::Ones(g->size(),1).array() - 
                           ( w.array() * D.array() ).rowwise().sum().array() ) ); // ng x ns+1

  if ( _logtrans )
  {
    uncert.array() = sqrt(exp(2.*values.array())*uncert.array().square());
    values.array() = exp(values.array())-1.0;
  }

  // apply detection limit
  for ( unsigned int i = 0; i < g->size(); i++ ) {

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
