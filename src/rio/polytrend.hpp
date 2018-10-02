#pragma once

#include <cmath>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "trendmodel.hpp"

namespace pt = boost::property_tree;

namespace rio
{

// class for a polynomial trend model with 1 proxy variable, the RIO standard in a way
class polytrend : public trendmodel
{
public:
  polytrend( const std::string& config_file );
  virtual ~polytrend();

  void select( const std::string& aggr, boost::posix_time::ptime tstart );
  
  void detrend( double& v, const std::string& station, const std::vector<double> proxy );

  void addtrend( double& v, double &e, const std::vector<double> proxy );

  // some advanced polynomial
  static double poly( const std::vector<double>& p, double x );
  static double poly( const std::vector<double>& p, double x, double x_lo, double x_hi );
  
private:
  pt::ptree   _trend;     //! boost property tree for trend parametes
  std::map<std::string, pt::ptree> _stat_param; //! statistical parameters for each station

  std::string _curr_sel;  //! string to the current aggregation selection for the parameters

  std::vector<double> _p_avg;
  std::vector<double> _p_avg_err;
  std::vector<double> _p_std;
  std::vector<double> _p_std_err;
  double              _ref_level_avg;
  double              _xlo_avg;
  double              _xhi_avg;
  double              _ref_level_std;
  double              _xlo_std;
  double              _xhi_std;
  
};

}
