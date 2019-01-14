#pragma once

#include "infra/cell.h"
#include "infra/geometadata.h"
#include "outputhandler.hpp"

#include <vector>

namespace rio {

struct griddefinition;

class memorywriter : public outputhandler
{
public:
    memorywriter(griddefinition gridDef);

    void init(const rio::config& cnf,
        const std::shared_ptr<rio::network> net,
        const std::shared_ptr<rio::grid> grid) override;

    void write(const boost::posix_time::ptime& curr_time,
        const std::unordered_map<std::string, double>& obs,
        const Eigen::VectorXd& values,
        const Eigen::VectorXd& uncert) override;

    void close(void) override;

    const inf::GeoMetadata& metadata() const;
    const std::vector<double>& data() const;

private:
    std::shared_ptr<rio::network> _net;
    std::shared_ptr<rio::grid> _grid;
    std::unordered_map<int64_t, inf::Cell> _gridMapping;

    inf::GeoMetadata _gridMeta;
    std::string _gridMapPattern;
    std::vector<double> _output;
};

}
