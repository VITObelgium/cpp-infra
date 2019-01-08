#include "pythonalgoadapters.h"
#include "maskedrasteralgo.h"
#include "pythonutils.h"
#include "rasterargument.h"

#include "gdx/algo/accuflux.h"
#include "gdx/algo/blurfilter.h"
#include "gdx/algo/cast.h"
#include "gdx/algo/category.h"
#include "gdx/algo/clusterid.h"
#include "gdx/algo/clustersize.h"
#include "gdx/algo/conditionals.h"
#include "gdx/algo/distance.h"
#include "gdx/algo/logical.h"
#include "gdx/algo/majorityfilter.h"
#include "gdx/algo/mathoperations.h"
#include "gdx/algo/maximum.h"
#include "gdx/algo/minimum.h"
#include "gdx/algo/nodata.h"
#include "gdx/algo/normalise.h"
#include "gdx/algo/random.h"
#include "gdx/algo/reclass.h"
#include "gdx/algo/shape.h"
#include "gdx/algo/sum.h"
#include "gdx/algo/suminbuffer.h"

#include "gdx/log.h"
#include "gdx/rastercompare.h"

#include <fmt/ostream.h>

namespace gdx {
namespace pyalgo {

namespace py = pybind11;
using namespace inf;

template <typename T>
using value_type = typename std::remove_cv_t<std::remove_reference_t<T>>::value_type;

namespace {

void throwOnInvalidClusterRaster(const Raster& raster)
{
    if (raster.type() != typeid(int32_t)) {
        throw InvalidArgument("Expected cluster raster to be of type int (numpy.dtype(int))");
    }
}

void throwOnRasterTypeMismatch(const Raster& raster1, const Raster& raster2)
{
    if (raster1.type() != raster2.type()) {
        throw InvalidArgument("Expected raster types to be the same ({} <-> {})", raster1.type().name(), raster2.type().name());
    }
}
}

Raster blurFilter(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::blurFilter(raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster majorityFilter(py::object rasterArg, double radiusInMeter)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::majorityFilter(raster, static_cast<float>(radiusInMeter)));
    },
        RasterArgument(rasterArg).variant());
}

template <typename RasterType>
std::vector<const RasterType*> createRasterVectorFromArgs(py::args args)
{
    try {
        std::vector<const Raster*> anyRasters;
        for (auto arg : args) {
            anyRasters.push_back(arg.cast<Raster*>());
        }

        using T = value_type<RasterType>;
        std::vector<const RasterType*> rasters;
        for (auto* ras : anyRasters) {
            rasters.push_back(&ras->get<T>());
        }

        return rasters;
    } catch (const std::bad_variant_access&) {
        throw InvalidArgument("Only raster objects of the same type are supported as argument");
    } catch (const py::cast_error&) {
        throw InvalidArgument("Only raster objects are supported as argument");
    }
}

Raster min(py::args args)
{
    if (args.size() == 0) {
        throw InvalidArgument("No raster objects provided");
    }

    auto* rasterPtr = args[0].cast<Raster*>();
    if (!rasterPtr) {
        throw InvalidArgument("Min arguments should be raster objects");
    }

    return std::visit([&args](auto&& raster) {
        return Raster(gdx::minimum(createRasterVectorFromArgs<std::remove_reference_t<decltype(raster)>>(args)));
    },
        rasterPtr->get());
}

Raster max(py::args args)
{
    if (args.size() == 0) {
        throw InvalidArgument("No raster objects provided");
    }

    auto* rasterPtr = args[0].cast<Raster*>();
    if (!rasterPtr) {
        throw InvalidArgument("Max arguments should be raster objects");
    }

    return std::visit([&args](auto&& raster) {
        return Raster(gdx::maximum(createRasterVectorFromArgs<std::remove_reference_t<decltype(raster)>>(args)));
    },
        rasterPtr->get());
}

Raster clip(pybind11::object rasterArg, double low, double high)
{
    return std::visit([&](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(gdx::clip(raster, static_cast<T>(low), static_cast<T>(high)));
    },
        RasterArgument(rasterArg).variant());
}

Raster clipLow(pybind11::object rasterArg, double low)
{
    return std::visit([&](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(gdx::clipLow(raster, static_cast<T>(low)));
    },
        RasterArgument(rasterArg).variant());
}

Raster clipHigh(pybind11::object rasterArg, double high)
{
    return std::visit([&](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(gdx::clipHigh(raster, static_cast<T>(high)));
    },
        RasterArgument(rasterArg).variant());
}

Raster abs(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::abs(raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster round(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::round(raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster sin(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::sin(raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster cos(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::cos(raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster log(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::log(raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster log10(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::log10(raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster exp(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::exp(raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster pow(py::object rasterArg1, py::object rasterArg2)
{
    RasterArgument r1(rasterArg1);
    RasterArgument r2(rasterArg2);

    throwOnRasterTypeMismatch(r1.raster(), r2.raster(r1.raster()));

    return std::visit([&](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(gdx::pow(raster, r2.get<T>(r1.raster())));
    },
        RasterArgument(rasterArg1).variant());
}

Raster normalise(pybind11::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::normalise<float>(raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster normaliseToByte(pybind11::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::normalise<uint8_t>(raster));
    },
        RasterArgument(rasterArg).variant());
}

bool any(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return gdx::any_of(raster);
    },
        RasterArgument(rasterArg).variant());
}

bool all(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return gdx::all_of(raster);
    },
        RasterArgument(rasterArg).variant());
}

py::object minimumValue(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return pybind11::cast(gdx::minimum(raster));
    },
        RasterArgument(rasterArg).variant());
}

py::object maximumValue(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return pybind11::cast(gdx::maximum(raster));
    },
        RasterArgument(rasterArg).variant());
}

double sum(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return gdx::sum(raster);
    },
        RasterArgument(rasterArg).variant());
}

Raster clusterSize(py::object rasterArg, bool includeDiagonal)
{
    auto diagonalSetting = includeDiagonal ? ClusterDiagonals::Include : ClusterDiagonals::Exclude;

    return std::visit([&](auto&& raster) {
        return Raster(gdx::clusterSize(raster, diagonalSetting));
    },
        RasterArgument(rasterArg).variant());
}

Raster clusterId(py::object rasterArg, bool includeDiagonal)
{
    auto diagonalSetting = includeDiagonal ? ClusterDiagonals::Include : ClusterDiagonals::Exclude;

    return std::visit([&](auto&& raster) {
        return Raster(gdx::clusterId(raster, diagonalSetting));
    },
        RasterArgument(rasterArg).variant());
}

Raster clusterIdWithObstacles(py::object rasterArg, py::object obstacleRasterArg)
{
    RasterArgument obstacleRaster(obstacleRasterArg);
    auto& obstacles = obstacleRaster.raster();
    if (obstacles.type() != typeid(uint8_t)) {
        throw InvalidArgument("Expected obstacle raster to be of type uint8 (numpy.dtype('B'))");
    }

    return std::visit([&](auto&& raster) {
        // Cathegories and obstacles have fixed types, so cast them
        auto cathegories = raster_cast<int32_t>(raster);
        return Raster(gdx::clusterIdWithObstacles(raster, cathegories, obstacles.get<uint8_t>()));
    },
        RasterArgument(rasterArg).variant());
}

Raster fuzzyClusterId(py::object rasterArg, float radius)
{
    return std::visit([&](auto&& raster) {
        showWarningIfClusteringOnFloats(raster);
        return Raster(gdx::fuzzyClusterId(raster, radius));
    },
        RasterArgument(rasterArg).variant());
}

Raster distance(py::object rasterArg)
{
    RasterArgument targetArg(rasterArg);
    auto& target = targetArg.raster();

    if (target.type() != typeid(uint8_t)) {
        throw InvalidArgument("Expected target raster to be of type uint8 (numpy.dtype('B')) actual type is {}", target.type().name());
    }

    return Raster(gdx::distance(target.get<uint8_t>()));
}

Raster travelDistance(py::object rasterArg, py::object anyTravelTimeArg)
{
    RasterArgument targetArg(rasterArg);
    auto& target = targetArg.raster();

    if (target.type() != typeid(uint8_t)) {
        throw InvalidArgument("Expected target raster to be of type uint8 (numpy.dtype('B')) actual type is {}", target.type().name());
    }

    return std::visit([&target](auto&& travelTime) {
        return Raster(gdx::travelDistance(target.get<uint8_t>(), travelTime));
    },
        RasterArgument(anyTravelTimeArg).variant());
}

Raster sumWithinTravelDistance(py::object anyMask, py::object anyResistance, py::object anyValuesMap, double maxResistance, bool includeAdjacent)
{
    return std::visit([maxResistance, includeAdjacent](auto&& mask, auto&& resistance, auto&& values) {
        return Raster(gdx::sumWithinTravelDistance(mask, resistance, values, static_cast<float>(maxResistance), includeAdjacent));
    },
        RasterArgument(anyMask).variant(), RasterArgument(anyResistance).variant(), RasterArgument(anyValuesMap).variant());
}

Raster closestTarget(py::object rasterTargetsArg)
{
    return std::visit([](auto&& target) {
        return Raster(gdx::closestTarget(target));
    },
        RasterArgument(rasterTargetsArg).variant());
}

Raster valueAtClosestTarget(py::object rasterTargetsArg, py::object valuesArg)
{
    return std::visit([](auto&& target, auto&& values) {
        return Raster(gdx::valueAtClosestTarget(target, values));
    },
        RasterArgument(rasterTargetsArg).variant(), RasterArgument(valuesArg).variant());
}

Raster valueAtClosestTravelTarget(py::object rasterTargetsArg, py::object travelTimeArg, py::object valuesArg)
{
    return std::visit([](auto&& target, auto&& travelTimes, auto&& values) {
        return Raster(gdx::valueAtClosestTravelTarget(target, travelTimes, values));
    },
        RasterArgument(rasterTargetsArg).variant(), RasterArgument(travelTimeArg).variant(), RasterArgument(valuesArg).variant());
}

Raster valueAtClosestLessThenTravelTarget(py::object rasterTargetsArg, py::object travelTimeArg, double maxTravelTime, py::object valuesArg)
{
    return std::visit([maxTravelTime](auto&& target, auto&& travelTimes, auto&& values) {
        return Raster(gdx::valueAtClosestLessThenTravelTarget(target, travelTimes, static_cast<float>(maxTravelTime), values));
    },
        RasterArgument(rasterTargetsArg).variant(), RasterArgument(travelTimeArg).variant(), RasterArgument(valuesArg).variant());
}

Raster categorySum(py::object clusterArg, py::object valuesArg)
{
    RasterArgument clusters(clusterArg);
    throwOnInvalidClusterRaster(clusters.raster());

    return std::visit([&](auto&& raster) {
        return Raster(gdx::categorySum(clusters.get<int32_t>(), raster));
    },
        RasterArgument(valuesArg).variant(clusters.raster()));
}

Raster categoryMin(py::object clusterArg, py::object valuesArg)
{
    RasterArgument clusters(clusterArg);
    throwOnInvalidClusterRaster(clusters.raster());

    return std::visit([&](auto&& raster) {
        return Raster(gdx::categoryMin(clusters.get<int32_t>(), raster));
    },
        RasterArgument(valuesArg).variant(clusters.raster()));
}

Raster categoryMax(py::object clusterArg, py::object valuesArg)
{
    RasterArgument clusters(clusterArg);
    throwOnInvalidClusterRaster(clusters.raster());

    return std::visit([&](auto&& raster) {
        return Raster(gdx::categoryMax(clusters.get<int32_t>(), raster));
    },
        RasterArgument(valuesArg).variant(clusters.raster()));
}

Raster categoryFilterOr(py::object clusterArg, py::object filterArg)
{
    RasterArgument clusters(clusterArg);
    throwOnInvalidClusterRaster(clusters.raster());

    return std::visit([&](auto&& raster) {
        return Raster(gdx::categoryFilterOr(clusters.get<int32_t>(), raster));
    },
        RasterArgument(filterArg).variant());
}

Raster categoryFilterAnd(py::object clusterArg, py::object filterArg)
{
    RasterArgument clusters(clusterArg);
    throwOnInvalidClusterRaster(clusters.raster());

    return std::visit([&](auto&& raster) {
        return Raster(gdx::categoryFilterAnd(clusters.get<int32_t>(), raster));
    },
        RasterArgument(filterArg).variant());
}

Raster categoryFilterNot(py::object clusterArg, py::object filterArg)
{
    RasterArgument clusters(clusterArg);
    throwOnInvalidClusterRaster(clusters.raster());

    return std::visit([&](auto&& raster) {
        return Raster(gdx::categoryFilterNot(clusters.get<int32_t>(), raster));
    },
        RasterArgument(filterArg).variant());
}

Raster categorySumInBuffer(py::object clusterArg, py::object valuesArg, double radiusInMeter)
{
    RasterArgument clusters(clusterArg);
    throwOnInvalidClusterRaster(clusters.raster());

    return std::visit([&](auto&& raster) {
        return Raster(gdx::categorySumInBuffer(clusters.get<int32_t>(), raster, static_cast<float>(radiusInMeter)));
    },
        RasterArgument(valuesArg).variant());
}

Raster sumInBuffer(const Raster& anyRaster, float radius)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::sumInBuffer(raster, radius));
    },
        anyRaster.get());
}

Raster maxInBuffer(const Raster& anyRaster, float radius)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::maxInBuffer(raster, radius));
    },
        anyRaster.get());
}

Raster reclass(const std::string& mappingFilepath, py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::reclass(mappingFilepath, raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster reclass(const std::string& mappingFilepath, py::object rasterArg1, py::object rasterArg2)
{
    RasterArgument r1(rasterArg1);
    RasterArgument r2(rasterArg2);

    return std::visit([&](auto&& raster1, auto&& raster2) {
        return Raster(gdx::reclass(mappingFilepath, raster1, raster2));
    },
        r1.variant(), r2.variant());
}

Raster reclass(const std::string& mappingFilepath, py::object rasterArg1, py::object rasterArg2, py::object rasterArg3)
{
    RasterArgument r1(rasterArg1);
    RasterArgument r2(rasterArg2);
    RasterArgument r3(rasterArg3);

    return std::visit([&](auto&& raster1, auto&& raster2, auto&& raster3) {
        return Raster(gdx::reclass(mappingFilepath, raster1, raster2, raster3));
    },
        r1.variant(), r2.variant(), r3.variant());
}

Raster reclassi(const std::string& mappingFilepath, py::object rasterArg, int32_t index)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::reclassi(mappingFilepath, raster, index));
    },
        RasterArgument(rasterArg).variant());
}

Raster reclassi(const std::string& mappingFilepath, py::object rasterArg1, py::object rasterArg2, int32_t index)
{
    RasterArgument r1(rasterArg1);
    RasterArgument r2(rasterArg2);

    return std::visit([&](auto&& raster1, auto&& raster2) {
        return Raster(gdx::reclassi(mappingFilepath, raster1, raster2, index));
    },
        r1.variant(), r2.variant());
}

Raster reclassi(const std::string& mappingFilepath, py::object rasterArg1, py::object rasterArg2, py::object rasterArg3, int32_t index)
{
    RasterArgument r1(rasterArg1);
    RasterArgument r2(rasterArg2);
    RasterArgument r3(rasterArg3);

    return std::visit([&](auto&& raster1, auto&& raster2, auto&& raster3) {
        return Raster(gdx::reclassi(mappingFilepath, raster1, raster2, raster3, index));
    },
        r1.variant(), r2.variant(), r3.variant());
}

Raster nreclass(const std::string& mappingFilepath, py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::nreclass(mappingFilepath, raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster logicalAnd(py::object rasterArg1, py::object rasterArg2)
{
    return RasterArgument(rasterArg1).raster() && RasterArgument(rasterArg2).raster();
}

Raster logicalAnd(py::object rasterArg1, py::object rasterArg2, py::object rasterArg3)
{
    return RasterArgument(rasterArg1).raster() && RasterArgument(rasterArg2).raster() && RasterArgument(rasterArg3).raster();
}

Raster logicalOr(py::object rasterArg1, py::object rasterArg2)
{
    return RasterArgument(rasterArg1).raster() || RasterArgument(rasterArg2).raster();
}

Raster logicalOr(py::object rasterArg1, py::object rasterArg2, py::object rasterArg3)
{
    return RasterArgument(rasterArg1).raster() || RasterArgument(rasterArg2).raster() || RasterArgument(rasterArg3).raster();
}

Raster logicalNot(py::object rasterArg)
{
    return !RasterArgument(rasterArg).raster();
}

Raster ifThenElse(py::object ifArg, py::object thenArg, py::object elseArg)
{
    RasterArgument ifRasterArg(ifArg);
    auto& ifRaster = ifRasterArg.raster();

    return std::visit([](auto&& rasterIf, auto&& rasterThen, auto&& rasterElse) -> Raster {
        return Raster(gdx::ifThenElse(rasterIf, rasterThen, rasterElse));
    },
        ifRaster.get(), RasterArgument(thenArg).variant(ifRaster.metadata()), RasterArgument(elseArg).variant(ifRaster.metadata()));
}

bool rasterEqual(py::object rasterArg1, py::object rasterArg2)
{
    RasterArgument r1(rasterArg1);
    auto& raster1 = r1.raster();

    return raster1.equalTo(RasterArgument(rasterArg2).raster(raster1));
}

Raster rasterEqualOneOf(py::object rasterArg, const std::vector<double>& values)
{
    return std::visit([&values](auto&& raster) -> Raster {
        using T = value_type<decltype(raster)>;
        std::vector<T> tvalues(values.size());
        std::transform(values.begin(), values.end(), tvalues.begin(), [](auto v) { return static_cast<T>(v); });
        return Raster(gdx::rasterEqualOneOf(raster, tvalues));
    },
        RasterArgument(rasterArg).variant());
}

bool allClose(py::object rasterArg1, py::object rasterArg2, double tolerance)
{
    RasterArgument r1(rasterArg1);
    auto& raster1 = r1.raster();

    return raster1.tolerant_data_equal_to(RasterArgument(rasterArg2).raster(raster1), tolerance);
}

Raster isClose(py::object rasterArg1, py::object rasterArg2, double relTolerance)
{
    RasterArgument r1(rasterArg1);
    auto& raster1 = r1.raster();

    RasterArgument r2(rasterArg2);
    auto& raster2 = r2.raster(raster1);

    throwOnRasterTypeMismatch(raster1, raster2);

    return std::visit([&raster2, relTolerance](auto&& raster1) -> Raster {
        using T = value_type<decltype(raster1)>;
        return Raster(gdx::isClose(raster1, raster2.get<T>(), static_cast<T>(relTolerance)));
    },
        raster1.get());
}

template <typename T>
static MaskedRaster<uint8_t> maskedis_nodata(const MaskedRaster<T>& input)
{
    MaskedRaster<uint8_t> output(input.metadata());
    gdx::is_nodata(input, output);
    return output;
}

Raster is_nodata(py::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        return Raster(maskedis_nodata(raster));
    },
        RasterArgument(rasterArg).variant());
}

Raster replaceValue(py::object rasterArg, py::object searchValue, py::object replaceValue)
{
    return std::visit([&](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(gdx::replaceValue(raster, searchValue.cast<T>(), replaceValue.cast<T>()));
    },
        RasterArgument(rasterArg).variant());
}

Raster replaceNodata(py::object rasterArg, py::object replaceValue)
{
    return std::visit([&](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return Raster(gdx::replace_nodata(raster, replaceValue.cast<T>()));
    },
        RasterArgument(rasterArg).variant());
}

void replaceNodataInPlace(Raster& raster, double replaceValue)
{
    std::visit([replaceValue](auto&& ras) {
        using T = value_type<decltype(ras)>;
        gdx::replace_nodata_in_place(ras, static_cast<T>(replaceValue));
    },
        raster.get());
}

void drawShapeFileOnRaster(Raster& anyRaster, const std::string& shapeFilepath)
{
    return std::visit([&](auto&& raster) {
        gdx::draw_shapefile_on_raster(raster, shapeFilepath);
    },
        anyRaster.get());
}

bool lddValidate(py::object rasterArg,
    const std::function<void(int32_t, int32_t)>& loopCb,
    const std::function<void(int32_t, int32_t)>& invalidValueCb,
    const std::function<void(int32_t, int32_t)>& endsInNodataCb,
    const std::function<void(int32_t, int32_t)>& outsideOfMapCb)
{
    return std::visit([&](auto&& raster) -> bool {
        using T = value_type<decltype(raster)>;
        if constexpr (std::is_same_v<T, uint8_t>) {
            return gdx::validateLdd(raster, loopCb, invalidValueCb, endsInNodataCb, outsideOfMapCb);
        } else {
            throw InvalidArgument("Ldd raster should be of type uint8_t");
        }
    },
        RasterArgument(rasterArg).variant());
}

Raster lddFix(py::object rasterArg)
{
    return std::visit([&](auto&& raster) -> Raster {
        using T = value_type<decltype(raster)>;
        if constexpr (std::is_same_v<T, uint8_t>) {
            auto& meta = raster.metadata();
            Point<int32_t> topLeft(int32_t(meta.xll), int32_t(meta.yll - meta.rows));

            std::set<Cell> errors;
            auto res = gdx::fixLdd(raster, errors);
            for (auto& cell : errors) {
                auto point = io::gdal::projected_to_geographic(31370, Point<double>(topLeft.x + cell.c, topLeft.y + cell.r));
                fmt::print(stderr, "Error cell: {}x{} -> {}x{}\n", cell.r, cell.c, point.x, point.y);
            }

            if (!errors.empty()) {
                throw RuntimeError("{} cells could not be fixed", errors.size());
            }

            return Raster(std::move(res));
        } else {
            throw InvalidArgument("Ldd raster should be of type uint8_t");
        }
    },
        RasterArgument(rasterArg).variant());
}

Raster accuflux(py::object lddArg, py::object freightArg)
{
    RasterArgument ldd(lddArg);
    auto& lddRaster = ldd.raster();

    if (lddRaster.type() != typeid(uint8_t)) {
        throw InvalidArgument("Expected ldd raster to be of type uint8 (numpy.dtype('B'))");
    }

    RasterArgument freight(freightArg);
    auto& freightRaster = freight.raster(lddRaster, typeid(float));

    if (freightRaster.type() != typeid(float)) {
        throw InvalidArgument("Expected freightMap raster to be of type float (numpy.dtype('float32'))");
    }

    return Raster(gdx::accuflux(lddRaster.get<uint8_t>(), freightRaster.get<float>()));
}

Raster accufractionflux(py::object lddArg, py::object freightArg, py::object fractionArg)
{
    RasterArgument ldd(lddArg);
    auto& lddRaster = ldd.raster();
    if (lddRaster.type() != typeid(uint8_t)) {
        throw InvalidArgument("Expected ldd raster to be of type uint8 (numpy.dtype('B'))");
    }

    RasterArgument freight(freightArg);
    auto& freightRaster = freight.raster(lddRaster, typeid(float));
    if (freightRaster.type() != typeid(float)) {
        throw InvalidArgument("Expected freight map raster to be of type float (numpy.dtype('float32'))");
    }

    RasterArgument fraction(fractionArg);
    auto& fractionRaster = fraction.raster(lddRaster, typeid(float));
    if (fractionRaster.type() != typeid(float)) {
        throw InvalidArgument("Expected fraction map raster to be of type float (numpy.dtype('float32'))");
    }

    return Raster(gdx::accufractionflux(lddRaster.get<uint8_t>(), freightRaster.get<float>(), fractionRaster.get<float>()));
}

Raster fluxOrigin(py::object lddArg, py::object freightArg, py::object fractionArg, py::object stationArg)
{
    RasterArgument ldd(lddArg);
    auto& lddRaster = ldd.raster();

    if (lddRaster.type() != typeid(uint8_t)) {
        throw InvalidArgument("Expected ldd raster to be of type uint8 (numpy.dtype('B'))");
    }

    RasterArgument freight(freightArg);
    auto& freightRaster = freight.raster(lddRaster, typeid(float));
    if (freightRaster.type() != typeid(float)) {
        throw InvalidArgument("Expected freight map raster to be of type float (numpy.dtype('float32'))");
    }

    RasterArgument fraction(fractionArg);
    auto& fractionRaster = fraction.raster(lddRaster, typeid(float));
    if (fractionRaster.type() != typeid(float)) {
        throw InvalidArgument("Expected fraction map raster to be of type float (numpy.dtype('float32'))");
    }

    RasterArgument station(stationArg);
    auto& stationRaster = fraction.raster();
    if (stationRaster.type() != typeid(int32_t)) {
        throw InvalidArgument("Expected station map raster to be of type int (numpy.dtype('int32'))");
    }

    return Raster(gdx::fluxOrigin(lddRaster.get<uint8_t>(), freightRaster.get<float>(), fractionRaster.get<float>(), stationRaster.get<int32_t>()));
}

Raster lddCluster(py::object lddArg, py::object idArg)
{
    RasterArgument ldd(lddArg);
    auto& lddRaster = ldd.raster();

    if (lddRaster.type() != typeid(uint8_t)) {
        throw InvalidArgument("Expected ldd raster to be of type uint8 (numpy.dtype('B'))");
    }

    RasterArgument ids(idArg);
    auto& idsRaster = ids.raster();
    if (idsRaster.type() != typeid(int32_t)) {
        throw InvalidArgument("Expected id map raster to be of type int (numpy.dtype('int32'))");
    }

    return Raster(gdx::lddCluster(lddRaster.get<uint8_t>(), idsRaster.get<int32_t>()));
}

Raster lddDist(py::object lddArg, py::object pointsArg, py::object frictionArg)
{
    RasterArgument ldd(lddArg);
    auto& lddRaster = ldd.raster();

    if (lddRaster.type() != typeid(uint8_t)) {
        throw InvalidArgument("Expected ldd raster to be of type uint8 (numpy.dtype('B'))");
    }

    RasterArgument points(pointsArg);
    auto& pointsRaster = points.raster();
    if (pointsRaster.type() != typeid(float)) {
        throw InvalidArgument("Expected points map raster to be of type float (numpy.dtype('float32'))");
    }

    RasterArgument friction(frictionArg);
    auto& frictionRaster = friction.raster(lddRaster, typeid(float));
    if (frictionRaster.type() != typeid(float)) {
        throw InvalidArgument("Expected friction map raster to be of type float (numpy.dtype('float32'))");
    }

    return Raster(gdx::lddDist(lddRaster.get<uint8_t>(), pointsRaster.get<float>(), frictionRaster.get<float>()));
}

Raster slopeLength(py::object lddArg, py::object frictionArg)
{
    RasterArgument ldd(lddArg);
    auto& lddRaster = ldd.raster();
    if (lddRaster.type() != typeid(uint8_t)) {
        throw InvalidArgument("Expected ldd raster to be of type uint8 (numpy.dtype('B'))");
    }

    RasterArgument friction(frictionArg);
    auto& frictionRaster = friction.raster(lddRaster, typeid(float));
    if (frictionRaster.type() != typeid(float)) {
        throw InvalidArgument("Expected friction map raster to be of type float (numpy.dtype('float32'))");
    }

    return Raster(gdx::slopeLength(lddRaster.get<uint8_t>(), frictionRaster.get<float>()));
}

Raster max_upstream_dist(py::object lddArg)
{
    RasterArgument ldd(lddArg);
    auto& lddRaster = ldd.raster();
    if (lddRaster.type() != typeid(uint8_t)) {
        throw InvalidArgument("Expected ldd raster to be of type uint8 (numpy.dtype('B'))");
    }

    return Raster(gdx::max_upstream_dist(lddRaster.get<uint8_t>()));
}

RasterStats<512> statistics(pybind11::object rasterArg)
{
    return std::visit([&](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return gdx::statistics<decltype(raster), 512>(raster, std::numeric_limits<T>::max());
    },
        RasterArgument(rasterArg).variant());
}

void tableRow(const std::string& output, pybind11::object rasterArg, pybind11::object categoryArg, Operation op, const std::string& label, bool append)
{
    RasterArgument r1(rasterArg);
    RasterArgument r2(categoryArg);

    throwOnRasterTypeMismatch(r1.raster(r2.raster()), r2.raster());

    return std::visit([&](auto&& raster) {
        using T = value_type<decltype(raster)>;
        return gdx::tableRow(raster, r2.get<T>(), op, output, label, append);
    },
        r1.variant(r2.raster()));
}

void randomFill(Raster& raster, double minValue, double maxValue)
{
    std::visit([minValue, maxValue](auto&& typedRaster) {
        using T = value_type<decltype(typedRaster)>;

        if (minValue < static_cast<double>(std::numeric_limits<T>::lowest())) {
            throw InvalidArgument("minimum value does not fit in the raster datatype ({} < {})", minValue, std::numeric_limits<T>::lowest());
        }

        if (maxValue > static_cast<double>(std::numeric_limits<T>::max())) {
            throw InvalidArgument("maximum value does not fit in the raster datatype ({} > {})", maxValue, std::numeric_limits<T>::max());
        }

        gdx::fillRandom(typedRaster, truncate<T>(minValue), truncate<T>(maxValue));
    },
        raster.get());
}
}
}
