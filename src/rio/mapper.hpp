#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <boost/date_time.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "conf.hpp"
#include "grid.hpp"
#include "network.hpp"

namespace pt = boost::property_tree;

namespace rio {

class mapper
{
public:
    mapper(const config& cf, const std::shared_ptr<network>& net);
    virtual ~mapper();

    virtual void run(Eigen::VectorXd& values,               // what is returned : vector corresponding to vector of gridcells
        Eigen::VectorXd& uncert,                            // uncertainty estimates
        boost::posix_time::ptime tstart,                    // the start time of the
        const std::unordered_map<std::string, double>& obs, // map of the station observations
        const std::shared_ptr<grid>& g) const = 0;          // the interpolation grid

    const std::string& name() const
    {
        return _name;
    }

protected:
    std::string _name;
    std::string _aggr;
    std::shared_ptr<network> _net;

    pt::ptree _opts;         //! general options from the <options> tag
    double _detection_limit; //! lower cut off
    double _missing_value;   //! hmmm
};

}
