#ifndef __MULTILINEARTRENDMODEL_H
#define __MULTILINEARTRENDMODEL_H

#include "TrendModel.h"

class MultiLinearTrendModel : public TrendModel {
public:
  MultiLinearTrendModel();
  ~MultiLinearTrendModel();
  
protected:
  Eigen::VectorXd avgTrend( const Eigen::MatrixXd &proxy ); // abstr fcn for trend on the avg
  Eigen::VectorXd stdTrend( const Eigen::MatrixXd &proxy ); // abstr fcn for trend on std
  Eigen::VectorXd avgTrendErr( const Eigen::MatrixXd &proxy );
  Eigen::VectorXd stdTrendErr( const Eigen::MatrixXd &proxy );
};

#endif /* #ifndef __LINEARTRENDMODEL_H */
