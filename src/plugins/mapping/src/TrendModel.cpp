#include <iostream>
#include <functional>

#include "TrendModel.h"

using namespace Eigen;

// little helper function
double limit_zero( double x ) {
  return ( x < 0 ? 0 : x ); 
}

TrendModel::TrendModel(){
}

TrendModel::~TrendModel(){
}


// generic detrending routine
void TrendModel::deTrend( VectorXd& C, VectorXd& Sig, const VectorXd& Avg, const MatrixXd &proxy ) {

  // in Avg we assume we have the station averages
  if ( ( C.size() != Sig.size() ) ||
       ( C.size() != Avg.size() ) ||
       ( C.size() != proxy.rows() ) ) {
    throw "LinearTrendModel::deTrend, Error in input sizes";
  }
  
  std::cout << "IMPLEMENT ME : EIGEN deTrend routine" << std::endl;
  /* comment out for now...

  // rescale according to trend in std deviation, these stdTrend and avgTrend should be
  // implemented by the daughter classes
  C = (C - Avg).array() * _std_ref / stdTrend( proxy ).array() + Avg;

  // apply the trend shift, delta_tr = avg_ref - p_avg * proxy
  C = C + _avg_ref - avgTrend(proxy).array();

  // put negative values to 0
  C = C.unaryExpr( std::fun_ptr( limit_zero ) );

  // here we leave Sig alone... 
  
  */ 

  return;
}


// generic trending adding routine
void TrendModel::addTrend( VectorXd& C, VectorXd& Sig, const MatrixXd &proxy ) {

  // in Avg we assume we have the station averages
  if ( ( C.size() != Sig.size() ) ||
       ( C.size() != proxy.rows() ) ) {
    throw "LinearTrendModel::addTrend, Error in input sizes";
  }

  // retrieve the trend function values
  VectorXd avg_tr = avgTrend(proxy);
  VectorXd std_tr = stdTrend(proxy);

  VectorXd avg_tr_err = avgTrendErr(proxy);
  VectorXd std_tr_err = stdTrendErr(proxy);


  std::cout << "IMPLEMENT ME : EIGEN addTrend routine" << std::endl;
  /*
    
  // fist compute uncertainty since we will be chaning the C vector
  // Sig should contain the Kriging error already
  Sig = sqrt( pow( Sig.array() / std_tr.array(), 2.) + pow( avg_tr_err.array(), 2  ) +
	      pow( std_tr_err.array() * ( C.array() - _avg_ref ) / _std_ref , 2. ) );
  
  // trend shift (avg trend)
  C = C - ( _avg_ref - avg_tr.array() );

  // rescale according to trend in std deviation
  C = ( C - avg_tr ).array() / ( _std_ref / std_tr.array() ) + avg_tr;
  
  // put negative values to 0
  C = C.unaryExpr( std::fun_ptr( limit_zero ) );

  */

  return;
}

//void TrendModel::updatePars( double *p_avg )
