#pragma once

#include <cmath>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "correlationmodel.hpp"

namespace pt = boost::property_tree;

namespace rio {

class exp_spcorr : public correlationmodel
{
public:
    exp_spcorr(const std::string& config_file);
    ~exp_spcorr();

    void select(const std::string& aggr, boost::posix_time::ptime tstart);

    double calc(double x1, double y1, double x2, double y2);

private:
    pt::ptree _param;      //! boost property tree for parametes
    std::string _curr_sel; //! string to the current aggregation selection for the parameters
    double _long_a;        //! the a parameter for the long range model rho = a * exp(-r/tau)
    double _long_tau;      //! the tau parameter for the long range model
    double _short_a;       //! the a parameter for the long range model rho = a * r + b
    double _short_b;       //! the tau parameter for the long range model
    double _range;         //! up till what distance to use the short range model
    bool _use_short;       //! use the short range model ?
};

}
