#pragma once

#include <string>
#include <boost/date_time.hpp>

namespace rio
{

class correlationmodel
{
public:
  correlationmodel( );
  virtual ~correlationmodel( );

  // virtual routine to select the correct parameters (e.g. depending on the
  // hour of the day)
  virtual void select( const std::string& aggr, boost::posix_time::ptime tstart ) = 0;
  
  // calcuates the spatial correlation
  virtual double calc( double x1, double y1, double x2, double y2 ) = 0;

  const std::string& name() const { return _name; }
  
protected:
  std::string _name; 
  
};


}
