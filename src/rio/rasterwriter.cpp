#include "rasterwriter.hpp"

#include "gdx/denseraster.h"
#include "gdx/denserasterio.h"
#include "gridmapper.hpp"
#include "infra/log.h"
#include "infra/xmldocument.h"
#include "parser.hpp"

#include <limits>

namespace rio {

using namespace inf;

rasterwriter::rasterwriter(const XmlNode& cnf)
: outputhandler(cnf)
, _outputPattern("rio_%timestamp%.tif")
{
    try {
        _outputPattern = _xml.get<std::string>("handler.location");
    } catch (...) {
        throw std::runtime_error("invalid configuration in rasterwriter XML config, <location> required");
    }
}

void rasterwriter::init(const rio::config& /*cnf*/,
    const std::shared_ptr<rio::network> /*net*/,
    const std::shared_ptr<rio::grid> grid)
{
    _grid = grid;

    auto mapFile = grid->definition().mapfilePattern;
    if (mapFile.empty()) {
        return;
    }

    rio::parser::get()->process(mapFile);

    _gridMapping     = read_mapping_file(fs::u8path(mapFile));
    _gridMeta        = grid->definition().metadata;
    _gridMeta.nodata = std::numeric_limits<double>::quiet_NaN();
}

void rasterwriter::write(const boost::posix_time::ptime& /*curr_time*/,
    const std::unordered_map<std::string, double>& /*obs*/,
    const Eigen::VectorXd& values,
    const Eigen::VectorXd& /*uncert*/)
{
    int valueIndex = -1;
    gdx::DenseRaster<double> raster(_gridMeta, _gridMeta.nodata.value_or(0.0));

    for (const auto& cell : _grid->cells()) {
        ++valueIndex;

        auto iter = _gridMapping.find(cell.id());
        if (iter == _gridMapping.end()) {
            Log::debug("No mapping for id: {}", cell.id());
            continue;
        }

        raster(iter->second.r, iter->second.c) = values[valueIndex];
    }

    std::string ofname = _outputPattern;
    rio::parser::get()->process(ofname);

    gdx::write_raster(raster, fs::u8path(ofname));
}

void rasterwriter::close()
{
}

}
