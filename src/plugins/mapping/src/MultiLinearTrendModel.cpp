#include <iostream>
#include "MultiLinearTrendModel.h"

MultiLinearTrendModel::MultiLinearTrendModel() 
  : TrendModel( ) {
  std::cout << "DO SOME MORE INITIALISATION HERE..." << std::endl;
}

MultiLinearTrendModel::~MultiLinearTrendModel() {
}

// the first column in the proxy variable needs to contain a column of ones if 
// there is a bias term present in the p_avg rowvector, 
Eigen::VectorXd MultiLinearTrendModel::avgTrend( const Eigen::MatrixXd& proxy ) {
  return (_p_avg * proxy);
}

Eigen::VectorXd MultiLinearTrendModel::stdTrend( const Eigen::MatrixXd& proxy ) {
  return (_p_std * proxy);
}

Eigen::VectorXd MultiLinearTrendModel::avgTrendErr( const Eigen::MatrixXd& proxy ) {
  return Eigen::VectorXd::Zero(proxy.size()); //not implemented yet
}

Eigen::VectorXd MultiLinearTrendModel::stdTrendErr( const Eigen::MatrixXd& proxy ) {
  return Eigen::VectorXd::Zero(proxy.size()); //not implemented yet
}
