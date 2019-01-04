#pragma once

#include "outputhandler.hpp"

#include <boost/date_time.hpp>
#include <string>
#include <vector>

namespace inf {
class XmlNode;
}

namespace rio {

class output
{
public:
    output(const inf::XmlNode& el, std::vector<std::string> req_outputs);
    virtual ~output();

    const std::vector<std::unique_ptr<rio::outputhandler>>& list()
    {
        return _list;
    }

    void init(const rio::config& cnf,
        const std::shared_ptr<rio::network> net,
        const std::shared_ptr<rio::grid> grid);

    void write(const boost::posix_time::ptime& curr_time,
        const std::unordered_map<std::string, double>& obs,
        const Eigen::VectorXd& values,
        const Eigen::VectorXd& uncert);

    void close();

private:
    std::vector<std::unique_ptr<rio::outputhandler>> _list;
};
}
