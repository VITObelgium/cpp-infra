#pragma once

#include "gdx/denseraster.h"
#include "infra/filesystem.h"
#include "infra/geometadata.h"
#include "infra/legend.h"

#include <mutex>
#include <qgeocoordinate.h>
#include <unordered_map>
#include <vector>

namespace opaq {

struct RasterDisplayData
{
    std::shared_ptr<gdx::DenseRaster<double>> raster;
    std::string colorMap;
    inf::Legend legend;
    double zoomLevel;
    QGeoCoordinate coordinate;
};

}
