#pragma once

#include <string>
#include <vector>
#include <boost/date_time.hpp>

namespace rio
{

// RIO trend model abstract base class
class trendmodel
{
public:
  trendmodel();
  virtual ~trendmodel();

  const std::string& name() const { return _name; }

public:
  virtual void select( const std::string& aggr, boost::posix_time::ptime tstart ) = 0;
  
  virtual void detrend( double& v, const std::string& station, const std::vector<double> proxy ) = 0;

  virtual void addtrend( double& v, double &e, const std::vector<double> proxy ) = 0;

protected:
  std::string _name;
};


}
