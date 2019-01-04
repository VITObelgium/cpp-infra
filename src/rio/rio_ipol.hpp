#pragma once

#include <cmath>

#include <Eigen/Core>
#include <Eigen/Dense>

#include "correlationmodel.hpp"
#include "mapper.hpp"
#include "network.hpp"
#include "trendmodel.hpp"

namespace rio {

class rio_ipol : public mapper
{
public:
    rio_ipol(const config& cf,
        const std::shared_ptr<rio::network>& net);

    virtual ~rio_ipol();

    void run(Eigen::VectorXd& values, // what is returned : vector corresponding to vector of gridcells
        Eigen::VectorXd& uncert,
        boost::posix_time::ptime tstart,                     // the start time of the
        const std::unordered_map<std::string, double>& obs,  // map of the station observations
        const std::shared_ptr<rio::grid>& g) const override; // the interpolation grid

private:
    std::unique_ptr<rio::correlationmodel> _spcorr;
    std::unique_ptr<rio::trendmodel> _trend;

    bool _logtrans;
};

}
