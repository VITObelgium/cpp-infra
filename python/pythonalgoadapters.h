#pragma once

#include "gdx/algo/statistics.h"
#include "gdx/algo/tablerow.h"
#include "gdx/raster.h"

#include <pybind11/pybind11.h>

namespace gdx::pyalgo {

Raster blurFilter(pybind11::object rasterArg);
Raster majorityFilter(pybind11::object rasterArg, double radiusInMeter);
Raster min(pybind11::args args);
Raster max(pybind11::args args);
Raster clip(pybind11::object rasterArg, double low, double high);
Raster clipLow(pybind11::object rasterArg, double low);
Raster clipHigh(pybind11::object rasterArg, double high);
Raster abs(pybind11::object rasterArg);
Raster round(pybind11::object rasterArg);
Raster sin(pybind11::object rasterArg);
Raster cos(pybind11::object rasterArg);
Raster log(pybind11::object rasterArg);
Raster log10(pybind11::object rasterArg);
Raster exp(pybind11::object rasterArg);
Raster pow(pybind11::object rasterArg1, pybind11::object rasterArg2);
Raster normalise(pybind11::object rasterArg);
Raster normaliseToByte(pybind11::object rasterArg);

bool any(pybind11::object rasterArg);
bool all(pybind11::object rasterArg);

pybind11::object minimumValue(pybind11::object rasterArg);
pybind11::object maximumValue(pybind11::object rasterArg);
double sum(pybind11::object rasterArg);

Raster clusterSize(pybind11::object rasterArg, bool includeDiagonal);
Raster clusterId(pybind11::object rasterArg, bool includeDiagonal);
Raster clusterIdWithObstacles(pybind11::object rasterArg, pybind11::object obstacleRasterArg);
Raster fuzzyClusterId(pybind11::object rasterArg, float radius);

Raster distance(pybind11::object rasterArg);
Raster travelDistance(pybind11::object rasterArg, pybind11::object anyTravelTime);
Raster sumWithinTravelDistance(pybind11::object anyMask, pybind11::object anyResistance, pybind11::object anyValuesMap, double maxResistance, bool includeAdjacent);
Raster closestTarget(pybind11::object rasterArg);
Raster valueAtClosestTarget(pybind11::object rasterArg, pybind11::object valuesArg);
Raster valueAtClosestTravelTarget(pybind11::object rasterArg, pybind11::object travelTimeArg, pybind11::object valuesArg);
Raster valueAtClosestLessThenTravelTarget(pybind11::object rasterArg, pybind11::object travelTimeArg, double maxTravelTime, pybind11::object valuesArg);

Raster categorySum(pybind11::object clusterArg, pybind11::object valuesArg);
Raster categoryMin(pybind11::object clusterArg, pybind11::object valuesArg);
Raster categoryMax(pybind11::object clusterArg, pybind11::object valuesArg);
Raster categoryFilterOr(pybind11::object clusterArg, pybind11::object filterArg);
Raster categoryFilterAnd(pybind11::object clusterArg, pybind11::object filterArg);
Raster categoryFilterNot(pybind11::object clusterArg, pybind11::object filterArg);
Raster categorySumInBuffer(pybind11::object clusterArg, pybind11::object valuesArg, double radiusInMeter);

Raster sumInBuffer(const Raster& anyRaster, float radius);
Raster maxInBuffer(const Raster& anyRaster, float radius);
Raster reclass(const std::string& mappingFilepath, pybind11::object rasterArg);
Raster reclass(const std::string& mappingFilepath, pybind11::object rasterArg1, pybind11::object rasterArg2);
Raster reclass(const std::string& mappingFilepath, pybind11::object rasterArg1, pybind11::object rasterArg2, pybind11::object rasterArg3);
Raster reclassi(const std::string& mappingFilepath, pybind11::object rasterArg, int32_t index);
Raster reclassi(const std::string& mappingFilepath, pybind11::object rasterArg1, pybind11::object rasterArg2, int32_t index);
Raster reclassi(const std::string& mappingFilepath, pybind11::object rasterArg1, pybind11::object rasterArg2, pybind11::object rasterArg3, int32_t index);
Raster nreclass(const std::string& mappingFilepath, pybind11::object rasterArg);

Raster logicalAnd(pybind11::object rasterArg1, pybind11::object rasterArg2);
Raster logicalAnd(pybind11::object rasterArg1, pybind11::object rasterArg2, pybind11::object rasterArg3);
Raster logicalOr(pybind11::object rasterArg1, pybind11::object rasterArg2);
Raster logicalOr(pybind11::object rasterArg1, pybind11::object rasterArg2, pybind11::object rasterArg3);
Raster logicalNot(pybind11::object rasterArg);
Raster ifThenElse(pybind11::object anyRasterIf, pybind11::object anyRasterThen, pybind11::object anyRasterElse);

bool rasterEqual(pybind11::object rasterArg1, pybind11::object rasterArg2);
Raster rasterEqualOneOf(pybind11::object rasterArg, const std::vector<double>& values);
bool allClose(pybind11::object rasterArg1, pybind11::object rasterArg2, double tolerance);
Raster isClose(pybind11::object rasterArg1, pybind11::object rasterArg2, double tolerance);

Raster is_nodata(pybind11::object rasterArg);
Raster replaceValue(pybind11::object rasterArg, pybind11::object searchValue, pybind11::object replaceValue);
Raster replaceNodata(pybind11::object rasterArg, pybind11::object replaceValue);
void replaceNodataInPlace(Raster& raster, double replaceValue);

void drawShapeFileOnRaster(Raster& anyRaster, const std::string& shapeFilepath);

bool lddValidate(pybind11::object rasterArg,
    const std::function<void(int32_t, int32_t)>& loopCb,
    const std::function<void(int32_t, int32_t)>& invalidValueCb,
    const std::function<void(int32_t, int32_t)>& endsInNoDataCb,
    const std::function<void(int32_t, int32_t)>& outsideOfMapCb);

Raster lddFix(pybind11::object lddArg);
Raster accuflux(pybind11::object lddArg, pybind11::object freightArg);
Raster accufractionflux(pybind11::object lddArg, pybind11::object freightArg, pybind11::object fractionArg);
Raster fluxOrigin(pybind11::object lddArg, pybind11::object freightArg, pybind11::object fractionArg, pybind11::object stationArg);
Raster lddCluster(pybind11::object lddArg, pybind11::object idArg);
Raster lddDist(pybind11::object lddArg, pybind11::object pointsArg, pybind11::object frictionArg);
Raster slopeLength(pybind11::object lddArg, pybind11::object frictionArg);
Raster max_upstream_dist(pybind11::object lddArg);

RasterStats<512> statistics(pybind11::object rasterArg);
void tableRow(const std::string& output, pybind11::object rasterArg, pybind11::object categoryArg, Operation op, const std::string& label, bool append);

void randomFill(Raster& raster, double minValue, double maxValue);
}
