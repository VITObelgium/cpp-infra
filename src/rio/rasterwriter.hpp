#pragma once

#include "infra/cell.h"
#include "infra/geometadata.h"
#include "outputhandler.hpp"

#include <vector>

namespace inf {
class XmlNode;
}

namespace rio {

struct griddefinition;

class rasterwriter : public outputhandler
{
public:
    rasterwriter(const inf::XmlNode& cnf);

    void init(const rio::config& cnf,
        const std::shared_ptr<rio::network> net,
        const std::shared_ptr<rio::grid> grid) override;

    void write(const boost::posix_time::ptime& curr_time,
        const std::unordered_map<std::string, double>& obs,
        const Eigen::VectorXd& values,
        const Eigen::VectorXd& uncert) override;

    void close(void) override;

private:
    std::shared_ptr<rio::grid> _grid;
    std::unordered_map<int64_t, inf::Cell> _gridMapping;

    inf::GeoMetadata _gridMeta;
    std::string _outputPattern;
    std::string _gridMapPattern;
};

}
