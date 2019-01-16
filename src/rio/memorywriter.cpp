#include "memorywriter.hpp"
#include "gridmapper.hpp"
#include "infra/log.h"
#include "parser.hpp"

namespace rio {

using namespace inf;

memorywriter::memorywriter(griddefinition gridDef)
: _gridMeta(gridDef.metadata)
, _gridMapPattern(gridDef.mapfilePattern)
{
}

void memorywriter::init(const rio::config& /*cnf*/,
    const std::shared_ptr<rio::network> net,
    const std::shared_ptr<rio::grid> grid)
{
    _net  = net;
    _grid = grid;

    _output.resize(_gridMeta.rows * _gridMeta.cols, _gridMeta.nodata.value_or(0.0));

    auto mapFile = _gridMapPattern;
    rio::parser::get()->process(mapFile);

    _gridMapping = read_mapping_file(fs::u8path(mapFile));
}

void memorywriter::write(const boost::posix_time::ptime& /*curr_time*/,
    const std::unordered_map<std::string, double>& obs,
    const Eigen::VectorXd& values,
    const Eigen::VectorXd& /*uncert*/)
{
    int valueIndex = -1;
    std::fill(_output.begin(), _output.end(), _gridMeta.nodata.value_or(0.0));
    for (const auto& cell : _grid->cells()) {
        ++valueIndex;

        auto iter = _gridMapping.find(cell.id());
        if (iter == _gridMapping.end()) {
            Log::debug("No mapping for id: {}", cell.id());
            continue;
        }

        auto gridIndex = iter->second.r * _gridMeta.cols + iter->second.c;
        assert(gridIndex <= _output.size());
        _output[gridIndex] = values[valueIndex];
    }

    _obs = obs;
}

void memorywriter::close()
{
}

const inf::GeoMetadata& memorywriter::metadata() const
{
    return _gridMeta;
}

const std::vector<double>& memorywriter::data() const
{
    return _output;
}

const std::unordered_map<std::string, double>& memorywriter::observations() const
{
    return _obs;
}

}
