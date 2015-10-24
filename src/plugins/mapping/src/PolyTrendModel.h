#ifndef __POLYTRENDMODEL_H
#define __POLYTRENDMODEL_H

#include <string>
#include "TrendModel.h"

class PolyTrendModel : public TrendModel {
public:
  PolyTrendModel( TiXmlElement *cnf );
  ~PolyTrendModel();
  
protected:
  Eigen::VectorXd avgTrend( const Eigen::MatrixXd &proxy ); // abstr fcn for trend on the avg
  Eigen::VectorXd stdTrend( const Eigen::MatrixXd &proxy ); // abstr fcn for trend on std
  Eigen::VectorXd avgTrendErr( const Eigen::MatrixXd &proxy );
  Eigen::VectorXd stdTrendErr( const Eigen::MatrixXd &proxy );

  int avgtr_order;       // polynomial order for avg trend
  int stdtr_order;       // idem for std trend...
  int avgtr_err_order;   // idem for avg err
  int stdtr_err_order;   // idem for std err

  double indic_lo_avg;
  double indic_hi_avg;

  double indic_lo_std;
  double indic_hi_std;
};

#endif /* #ifndef __LINEARTRENDMODEL_H */
