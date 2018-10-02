#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <tinyxml.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "conf.hpp"
#include "network.hpp"
#include "grid.hpp"

namespace pt = boost::property_tree;

namespace rio
{

class outputhandler
{
public:
  outputhandler( TiXmlElement *cnf );
  virtual ~outputhandler();

  virtual void init( const rio::config& cnf,
		     const std::shared_ptr<rio::network> net,
		     const std::shared_ptr<rio::grid> grid ) = 0;

  virtual void write( const boost::posix_time::ptime& curr_time, 
                      const std::map<std::string, double>& obs, 
                      const Eigen::VectorXd& values,
                      const Eigen::VectorXd& uncert ) = 0;
 
  virtual void close( void ) = 0;

protected:
  pt::ptree _xml;
};

}
