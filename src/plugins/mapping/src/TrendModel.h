#ifndef __TRENDMODEL_H
#define __TRENDMODEL_H

#include "tinyxml.h"
#include <Eigen/Dense>

class TrendModel {
public:
  TrendModel( );
  virtual ~TrendModel();
  
  // abstract function which converts the concentration vector and sigma vector
  // into a detrended one, given a matrix of proxy parameters for the corresponding
  // vectors of concentrations
  void deTrend( Eigen::VectorXd& C, Eigen::VectorXd& Sig, 
		const Eigen::VectorXd& Avg, const Eigen::MatrixXd &proxy );

  void addTrend( Eigen::VectorXd& C, Eigen::VectorXd& Sig, 
		 const Eigen::MatrixXd &proxy );

protected:
  virtual Eigen::VectorXd avgTrend( const Eigen::MatrixXd &proxy ) = 0; // abstr fcn for trend on the avg
  virtual Eigen::VectorXd stdTrend( const Eigen::MatrixXd &proxy ) = 0; // abstr fcn for trend on std
  virtual Eigen::VectorXd avgTrendErr( const Eigen::MatrixXd &proxy ) = 0;
  virtual Eigen::VectorXd stdTrendErr( const Eigen::MatrixXd &proxy ) = 0;
  
  double _avg_ref; // trend model reference value for avg of concentration
  double _std_ref; // trend model reference value for std of concentration
  
  Eigen::RowVectorXd _p_avg;     // parameters for trend on concentration averages
  Eigen::RowVectorXd _p_avg_err; // parameters for 1 sigma error band on averages

  Eigen::RowVectorXd _p_std;     // parameters for trend on concentration std
  Eigen::RowVectorXd _p_std_err; // parameters for 1 sigma error band on std
};

#endif /* #ifndef __TRENDMODEL_H */
