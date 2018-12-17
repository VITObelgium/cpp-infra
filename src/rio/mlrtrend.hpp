#pragma once

#include <cmath>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "trendmodel.hpp"

namespace pt = boost::property_tree;

namespace rio
{

// class for a polynomial trend model with 1 proxy variable, the RIO standard in a way
class mlrtrend : public trendmodel
{
public:
  mlrtrend( const std::string& config_file );
  virtual ~mlrtrend();

  void select( const std::string& aggr, boost::posix_time::ptime tstart );
  
  void detrend( double& v, const std::string& station, const std::vector<double> proxy );

  void addtrend( double& v, double &e, const std::vector<double> proxy );
  

protected:
    // function to calculate the regression result
    // c   : the regression coefficients 
    // x   : the proxy parameters, incl 1 if the model contains a constant term..
    // xlo : lower limits for the proxy value in this dimension( kept to xlo[i] when x[i] < xlo[i]
    // xhi : upper limits for the proxy value in this dimension( kept to xhi[i] when x[i] > xhi[i]
    double mlr_apply(const std::vector<double>& c, const std::vector<double>& x, const std::vector<double>& xlo, const std::vector<double>& xhi);

private:
  pt::ptree   _trend;     //! boost property tree for trend parametes
  std::map<std::string, pt::ptree> _stat_param; //! statistical parameters for each station

  std::string _curr_sel;  //! string to the current aggregation selection for the parameters

  std::vector<double> _p_avg;
  std::vector<double> _p_std;
  
  double              _ref_level_avg;
  std::vector<double> _xlo_avg;
  std::vector<double> _xhi_avg;
  
  double              _ref_level_std;
  std::vector<double> _xlo_std;
  std::vector<double> _xhi_std;
  
};

}
