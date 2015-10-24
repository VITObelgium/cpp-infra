#include <math.h>
#include <string>
#include <limits>

#include "PolyTrendModel.h"

// Little helper ruotine to construct a vandermonde matrix of given order
Eigen::MatrixXd Vandermonde( const Eigen::VectorXd& a, int order ) {

  Eigen::MatrixXd V(a.rows(),order+1); 
  V.array().col(0) = Eigen::VectorXd::Ones(a.rows()); // set first column to 1 for bias term
  for( int i=1;i<order+1;++i) V.array().col(i) = pow(a.array(),i);
  
  return V;
}

PolyTrendModel::PolyTrendModel( TiXmlElement *cnf ) 
  : TrendModel( ) {

  TiXmlElement *pEl;
  std::string type;
  
  pEl = cnf->FirstChildElement( "avgtrend" );
  if ( ! pEl ) throw "PolyTrendModel::PolyTrendModel, cannot find avgtrend element in cnf";
  if ( pEl->QueryStringAttribute( "type", &type ) != TIXML_SUCCESS )
    throw "PolyTrendModel::PolyTrendModel, cannot find avgtrend type attribute"; 
  if ( pEl->QueryDoubleAttribute( "ref_level", &_avg_ref ) != TIXML_SUCCESS )
    throw "PolyTrendModel::PolyTrendModel, cannot find avgtrend type attribute"; 
  if ( pEl->QueryDoubleAttribute( "indic_lo", &indic_lo_avg ) != TIXML_SUCCESS ) indic_lo_avg = std::numeric_limits<double>::min(); 
  if ( pEl->QueryDoubleAttribute( "indic_hi", &indic_hi_avg ) != TIXML_SUCCESS ) indic_hi_avg = std::numeric_limits<double>::max();
  
  // we leave it here like this to be compatible with previous RIO versions
  if ( type == "poly1" ) {
    avgtr_order     = 1;
    avgtr_err_order = 2;
  } else if ( type == "poly2" ) {
    avgtr_order     = 2;
    avgtr_err_order = 4;
  } else throw "PolyTrendModel::PolyTrendModel, only trends up to 2nd order supported (avg)";

  
  pEl = cnf->FirstChildElement( "stdtrend" );
  if ( ! pEl ) throw "PolyTrendModel::PolyTrendModel, cannot find stdtrend element in cnf";
  if ( pEl->QueryStringAttribute( "type", &type ) != TIXML_SUCCESS )
    throw "PolyTrendModel::PolyTrendModel, cannot find avgtrend type attribute"; 
  if ( pEl->QueryDoubleAttribute( "ref_level", &_std_ref ) != TIXML_SUCCESS )
    throw "PolyTrendModel::PolyTrendModel, cannot find avgtrend type attribute"; 
  if ( pEl->QueryDoubleAttribute( "indic_lo", &indic_lo_std ) != TIXML_SUCCESS ) indic_lo_std = std::numeric_limits<double>::min(); 
  if ( pEl->QueryDoubleAttribute( "indic_hi", &indic_hi_std ) != TIXML_SUCCESS ) indic_hi_std = std::numeric_limits<double>::max();
  
  // we leave it here like this to be compatible with previous RIO versions
  if ( type == "poly1" ) {
    stdtr_order     = 1;
    stdtr_err_order = 2;
  } else if ( type == "poly2" ) {
    stdtr_order     = 2;
    stdtr_err_order = 4;
  } else throw "PolyTrendModel::PolyTrendModel, only trends up to 2nd order supported (std)";

  
  // now size up the matrix elements
  _p_avg.resize( avgtr_order + 1 );
  _p_std.resize( stdtr_order + 1 );

  _p_avg_err.resize( avgtr_err_order + 1 );
  _p_std_err.resize( stdtr_err_order + 1 );
}

PolyTrendModel::~PolyTrendModel() {
  
}

// the first column in the proxy variable needs to contain a column of ones if 
// there is a bias term present in the p_avg rowvector, 
Eigen::VectorXd PolyTrendModel::avgTrend( const Eigen::MatrixXd& proxy ) {
  if ( proxy.cols() != 1 ) throw "polynomial trend model expects single vector input";
  return _p_avg * Vandermonde( proxy, avgtr_order );
}

Eigen::VectorXd PolyTrendModel::stdTrend( const Eigen::MatrixXd& proxy ) {
  if ( proxy.cols() != 1 ) throw "polynomial trend model expects single vector input";
  return _p_std * Vandermonde( proxy, stdtr_order );
}

Eigen::VectorXd PolyTrendModel::avgTrendErr( const Eigen::MatrixXd& proxy ) {
  if ( proxy.cols() != 1 ) throw "polynomial trend model expects single vector input";
  return _p_avg_err * Vandermonde( proxy, avgtr_err_order );
}

Eigen::VectorXd PolyTrendModel::stdTrendErr( const Eigen::MatrixXd& proxy ) {
  if ( proxy.cols() != 1 ) throw "polynomial trend model expects single vector input";
  return _p_avg_err * Vandermonde( proxy, stdtr_err_order );
}
