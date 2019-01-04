#pragma once

#include <boost/date_time.hpp>
#include <map>
#include <memory>

#include "network.hpp"

namespace rio {

class obshandler
{
public:
    obshandler();
    virtual ~obshandler();

    void setNetwork(std::shared_ptr<rio::network const> net)
    {
        _net = net;
    }

    virtual std::unordered_map<std::string, double> get(boost::posix_time::ptime tstart, std::string pol, std::string agg) = 0;

protected:
    std::shared_ptr<rio::network const> _net;
};

}
