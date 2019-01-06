#pragma once

#include <unordered_map>
#include <stdexcept>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "conf.hpp"
#include "grid.hpp"
#include "network.hpp"

namespace pt = boost::property_tree;

namespace inf {
class XmlNode;
}

namespace rio {

class outputhandler
{
public:
    outputhandler(const inf::XmlNode& cnf);
    virtual ~outputhandler() = default;

    virtual void init(const rio::config& cnf,
        const std::shared_ptr<rio::network> net,
        const std::shared_ptr<rio::grid> grid) = 0;

    virtual void write(const boost::posix_time::ptime& curr_time,
        const std::unordered_map<std::string, double>& obs,
        const Eigen::VectorXd& values,
        const Eigen::VectorXd& uncert) = 0;

    virtual void close(void) = 0;

protected:
    pt::ptree _xml;
};
}
