#pragma once

#include "mapper.hpp"

namespace rio {

class idw_ipol : public mapper
{
public:
    idw_ipol(const config& cf, const std::shared_ptr<rio::network>& net);
    ~idw_ipol();

    void run(Eigen::VectorXd& values,                       // what is returned : vector corresponding to vector of gridcells
        Eigen::VectorXd& uncert,                            // uncertainty estimates
        boost::posix_time::ptime tstart,                    // the start time of the
        const std::unordered_map<std::string, double>& obs, // map of the station observations
        const std::shared_ptr<rio::grid>& g) const;         // the interpolation grid

private:
    double _p; // distance power
};

}
