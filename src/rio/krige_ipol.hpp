#pragma once

#include <Eigen/Core>
#include <Eigen/Dense>

#include "mapper.hpp"
#include "network.hpp"
#include "correlationmodel.hpp"

namespace rio
{

class krige_ipol : public mapper
{
public:
  krige_ipol( const config& cf,
	      const std::shared_ptr<rio::network>& net );
  virtual ~krige_ipol();

  void run( Eigen::VectorXd& values,          // what is returned : vector corresponding to vector of gridcells 
            Eigen::VectorXd& uncert, 
            boost::posix_time::ptime tstart,  // the start time of the 
            const std::map<std::string, double>& obs, // map of the station observations
            const std::shared_ptr<rio::grid>& g ) const;            // the interpolation grid

private:
  std::shared_ptr<rio::correlationmodel> _spcorr;
  bool _logtrans;
};
  

}
