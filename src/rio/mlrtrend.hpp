#pragma once

#include <cmath>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "trendmodel.hpp"

namespace pt = boost::property_tree;

namespace rio {

// class for a multiple linear regression trend model
class mlrtrend : public trendmodel
{
public:
    mlrtrend(const std::string& config_file);
    virtual ~mlrtrend();

    void select(const std::string& aggr, boost::posix_time::ptime tstart);

    void detrend(double& v, const std::string& station, const std::vector<double> proxy);

    void addtrend(double& v, double& e, const std::vector<double> proxy);

private:
};

} //namespace rio
