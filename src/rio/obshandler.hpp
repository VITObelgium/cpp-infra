#pragma once

#include <boost/date_time.hpp>
#include <memory>
#include <unordered_map>

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
    virtual boost::posix_time::ptime first_time() const                                                                    = 0;
    virtual boost::posix_time::ptime last_time() const                                                                     = 0;

    std::shared_ptr<const rio::network> network() const
    {
        return _net;
    }

protected:
    std::shared_ptr<const rio::network> _net;
};

}
