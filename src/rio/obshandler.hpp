#pragma once

#include <map>
#include <memory>
#include <boost/date_time.hpp>

#include "network.hpp"

namespace rio
{

class obshandler
{
public:
  obshandler();
  virtual ~obshandler();

  void setNetwork( std::shared_ptr<rio::network const> net ) { _net = net; }
    
  
  virtual void get( std::map<std::string, double>& data,
		    boost::posix_time::ptime tstart, std::string pol, std::string agg ) = 0;

protected:
  std::shared_ptr<rio::network const> _net; 

};

}
