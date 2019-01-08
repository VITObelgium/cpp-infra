#include <pybind11/eigen.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#ifndef PYBIND11_CPP17
static_assert(false, "pybind11 did not detect c++17 support");
#endif

#include "gdx/exception.h"
#include "gdx/log.h"
#include "gdx/point.h"
#include "gdx/rasterio.h"
#include "gdxconfig.h"
#include "infra/gdal.h"
#include "infra/gdalalgo.h"

#include "pythonadapters.h"
#include "pythonalgoadapters.h"
#include "pythonioadapters.h"
#include "pythonlogsink.h"
#include "pythonutils.h"
#include "rasterargument.h"

#include "gdx/maskedraster.h"

#include <fmt/format.h>
#include <gsl/span>

namespace py = pybind11;
using namespace py::literals;

PYBIND11_MODULE(gdx, mod)
{
    using namespace gdx;

    Log::add_custom_sink(std::make_shared<gdx::python::LogSinkMt>());
    Log::initialize("gdx");
    inf::gdal::registerGdal();

#ifndef NDEBUG
    Log::warn("GDX IN DEBUG MODE");
#endif

    auto atexit = py::module::import("atexit");
    atexit.attr("register")(py::cpp_function([]() {
        // perform cleanup here, this function is called with the GIL held
        inf::gdal::unregisterGdal();
        Log::uninitialize();
    }));

    mod.attr("__version__") = fmt::format("{} ({})", GDX_VERSION, GDX_COMMIT_HASH);

    auto cmMod        = mod.def_submodule("cm");
    cmMod.attr("ldd") = gdx::cm::ldd;

    py::class_<gdx::NodataType>(mod, "NodataType");
    mod.attr("nodata") = gdx::NodataType();

    py::enum_<Log::Level>(mod, "log_level")
        .value("debug", Log::Level::Debug)
        .value("info", Log::Level::Info)
        .value("warning", Log::Level::Warning)
        .value("error", Log::Level::Error)
        .value("critical", Log::Level::Critical);

    py::enum_<inf::gdal::ResampleAlgorithm>(mod, "resample_algo")
        .value("nearest_neighbour", inf::gdal::ResampleAlgorithm::NearestNeighbour)
        .value("bilinear", inf::gdal::ResampleAlgorithm::Bilinear)
        .value("cubic", inf::gdal::ResampleAlgorithm::Cubic)
        .value("cubic_spline", inf::gdal::ResampleAlgorithm::CubicSpline)
        .value("lanczos", inf::gdal::ResampleAlgorithm::Lanczos)
        .value("average", inf::gdal::ResampleAlgorithm::Average)
        .value("mode", inf::gdal::ResampleAlgorithm::Mode)
        .value("maximum", inf::gdal::ResampleAlgorithm::Maximum)
        .value("minimum", inf::gdal::ResampleAlgorithm::Minimum)
        .value("median", inf::gdal::ResampleAlgorithm::Median)
        .value("first_quantile", inf::gdal::ResampleAlgorithm::FirstQuantile)
        .value("third_quantile", inf::gdal::ResampleAlgorithm::ThirdQuantile);

    py::enum_<Operation>(mod, "operation")
        .value("average", Operation::Average)
        .value("minimum", Operation::Min)
        .value("maximum", Operation::Max)
        .value("sum", Operation::Sum)
        .value("count", Operation::Count)
        .value("count_non_zero", Operation::CountNonZero);

    py::class_<gdx::RasterMetadata>(mod, "raster_metadata", "Raster metadata data structure.")
        .def(py::init<>())
        .def(py::init<const gdx::RasterMetadata&>())
        .def(py::init<int32_t, int32_t, double, double, double, std::optional<double>>(), py::arg("rows"), py::arg("cols"), py::arg("xll") = 0.0, py::arg("yll") = 0.0, py::arg("cell_size") = 100.0, py::arg("nodata") = std::optional<double>())
        .def("__repr__", [](const gdx::RasterMetadata& meta) {
            return fmt::format("<gdx.raster_metadata {}x{}>", meta.rows, meta.cols);
        })
        .def("__str__", &gdx::RasterMetadata::to_string)
        .def("_repr_html_", &showMetadata)
        .def("set_projection_from_epsg", &gdx::RasterMetadata::set_projection_from_epsg)
        .def_property_readonly("bounds_geo", [](const gdx::RasterMetadata& self) { return rasterBounds(self, false); })
        .def_property_readonly("bounds", [](const gdx::RasterMetadata& self) { return rasterBounds(self, true); })
        .def_readwrite("rows", &gdx::RasterMetadata::rows)
        .def_readwrite("cols", &gdx::RasterMetadata::cols)
        .def_readwrite("cell_size", &gdx::RasterMetadata::cellSize)
        .def_readwrite("xll", &gdx::RasterMetadata::xll)
        .def_readwrite("yll", &gdx::RasterMetadata::yll)
        .def_readwrite("nodata", &gdx::RasterMetadata::nodata)
        .def_readwrite("projection", &gdx::RasterMetadata::projection)
        .def(
            "coord_to_array_index", [](const gdx::RasterMetadata& meta, double x, double y) {
                return py::make_tuple(meta.convert_y_to_row(y), meta.convert_x_to_col(x));
            },
            py::arg("x"), py::arg("y"), "Return the (row, col) tuple of the cell that contains the (x, y) coordinate")
        .def(
            "array_index_to_coord", [](const gdx::RasterMetadata& meta, int row, int col) {
                return py::make_tuple(meta.convert_col_centre_to_x(col), meta.convert_row_centre_to_y(row));
            },
            py::arg("row"), py::arg("col"), "Converts the center of the cell at (row, col) to a (x, y) tuple");

    py::class_<gdx::RasterStats<512>>(mod, "raster_statistics", "Raster statistics data structure.")
        .def(py::init<>())
        .def("__repr__", [](const gdx::RasterStats<512>&) {
            return "<gdx.raster_statistics>";
        })
        .def("__str__", &gdx::RasterStats<512>::toString)
        .def("_repr_html_", &showRasterStats)
        .def_readonly("negative_values", &gdx::RasterStats<512>::negativeValues)
        .def_readonly("count_high", &gdx::RasterStats<512>::countHigh)
        .def_readonly("non_zero_values", &gdx::RasterStats<512>::nonZeroValues)
        .def_readonly("zero_values", &gdx::RasterStats<512>::zeroValues)
        .def_readonly("nodata_values", &gdx::RasterStats<512>::noDataValues)
        .def_readonly("nan_values", &gdx::RasterStats<512>::nanValues)
        .def_readonly("sum", &gdx::RasterStats<512>::sum)
        .def_readonly("sigmaNonZero", &gdx::RasterStats<512>::sigmaNonZero)
        .def_readonly("max_value", &gdx::RasterStats<512>::highestValue)
        .def_readonly("min_value", &gdx::RasterStats<512>::lowestValue);

    py::class_<Raster>(mod, "raster")
        // Customized init funtions because we need to convert the dataType first
        .def(py::init([](int32_t rows, int32_t cols, py::object dataType) {
            return Raster(rows, cols, dtypeToRasterType(py::dtype::from_args(dataType)));
        }),
            py::arg("rows"), py::arg("cols"), py::arg("dtype") = py::dtype::of<float>())
        .def(py::init([](int32_t rows, int32_t cols, py::object dataType, double assignValue) {
            return Raster(rows, cols, dtypeToRasterType(py::dtype::from_args(dataType)), assignValue);
        }),
            py::arg("rows"), py::arg("cols"), py::arg("dtype") = py::dtype::of<float>(), py::arg("fill"))
        .def(py::init([](const RasterMetadata& meta, py::object dataType) {
            return Raster(meta, dtypeToRasterType(py::dtype::from_args(dataType)));
        }),
            py::arg("metadata"), py::arg("dtype") = py::dtype::of<float>())
        .def(py::init([](const RasterMetadata& meta, py::object dataType, double assignValue) {
            return Raster(meta, dtypeToRasterType(py::dtype::from_args(dataType)), assignValue);
        }),
            py::arg("metadata"), py::arg("dtype") = py::dtype::of<float>(), py::arg("fill"))
        .def("__repr__", &rasterRepresentation)
        .def("_repr_html_", [](py::object instance) {
            // use the representation of the foilum map
            return showRaster(instance, py::none(), true).attr("_repr_html_")();
        })
        .def("set_projection", &Raster::set_projection)
        .def("replace_value", &Raster::replaceValue, "old"_a, "new"_a, "Replaces all occurences of 'old' value with 'new' value")
        .def("set_value", &Raster::setValue, "row"_a, "col"_a, "value"_a, "Assigns the value to the raster cell at [row][col]")
        .def_property_readonly("metadata", &Raster::metadata, py::return_value_policy::reference_internal)
        .def_property_readonly("dtype", [](const Raster& ras) { return rasterTypeToDtype(ras.type()); })
        .def_property_readonly(
            "array", [](Raster& raster) {
                return rasterNumpyMaskedArray(raster);
            },
            py::return_value_policy::reference_internal)

        .def(py::self + py::self)
        .def(py::self + int32_t())
        .def(py::self + double())
        .def(int32_t() + py::self)
        .def(double() + py::self)
        .def(py::self - py::self)
        .def(py::self - int32_t())
        .def(py::self - double())
        .def(int32_t() - py::self)
        .def(double() - py::self)
        .def(py::self * py::self)
        .def(py::self * int32_t())
        .def(py::self * double())
        .def(int32_t() * py::self)
        .def(double() * py::self)
        .def(py::self / py::self)
        .def(py::self / int32_t())
        .def(py::self / double())
        .def(int32_t() / py::self)
        .def(double() / py::self)
        .def(py::self == py::self)
        .def(py::self == int32_t())
        .def(py::self == double())
        .def(py::self != py::self)
        .def(py::self != int32_t())
        .def(py::self != double())
        .def(py::self < py::self)
        .def(py::self <= py::self)
        .def(py::self > py::self)
        .def(py::self >= py::self)
        .def(py::self < double())
        .def(py::self <= double())
        .def(py::self > double())
        .def(py::self >= double())
        .def(-py::self)
        .def("__bool__", &Raster::operator bool)
        .def("__copy__", [](const Raster& ras) {
            return ras.copy();
        })
        .def("__deepcopy__", [](const Raster& ras, py::dict /*dict*/) {
            // we do not need to update the dict as a raster does not own other python objects
            // so we will never encounter a recursive copy
            return ras.copy();
        })
        .def(
            "astype", [](const Raster& raster, py::object type) {
                return castRaster(raster, type);
            },
            "Returns a copy of the raster with the requested type.\n"
            "Notice:\n"
            "In case of a floating point raster with NaN nodata\n"
            "When casting to an unsigned value the nodata value becomes 0\n"
            "When casting to a signed value the nodata value becomes the largest negative number.")
        .def("fill", &Raster::fill, "Assign the specified value to the entire raster.")
        .def(
            "fill_random", [](Raster& raster, double minValue, double maxValue) { pyalgo::randomFill(raster, minValue, maxValue); }, "min_value"_a = 0.0, "max_value"_a = 1.0, "Fill the raster with random data in the range [min_value, max_value]")
        .def(
            "replace_nodata", [](Raster& raster, double value) { pyalgo::replaceNodataInPlace(raster, value); }, "value"_a, "Replaces the nodata values of the raster with the given value, after this operation there will be no nodata values");

    // Free functions

    mod.def("set_log_level",
        &Log::setLevel,
        "Set the log level (default = Warning)");

    mod.def("read",
        py::overload_cast<py::object>(&read_raster),
        "raster_path"_a,
        "Read a raster from disk");

    mod.def("read",
        py::overload_cast<py::object, const RasterMetadata&>(&read_raster),
        "raster_path"_a,
        "extent"_a,
        "Read a raster from disk");

    mod.def("read_as",
        py::overload_cast<py::object, py::object>(&read_raster),
        "dtype"_a,
        "raster_path"_a,
        "Read a raster from disk and convert to the requested format");

    mod.def("write",
        py::overload_cast<Raster&, const std::string&, py::object>(&write_raster),
        "raster"_a,
        "raster_path"_a,
        "color_map"_a = py::none(),
        "Write the provided raster to disk");

    mod.def("write_as",
        py::overload_cast<py::object, Raster&, const std::string&, py::object>(&write_raster),
        "dtype"_a,
        "raster"_a,
        "raster_path"_a,
        "color_map"_a = py::none(),
        "Write the provided raster to disk and convert to the requested format");

    mod.def("all",
        &pyalgo::all,
        "raster"_a,
        "Returns True if all elements in the raster evaluate to True");

    mod.def("any",
        &pyalgo::any,
        "raster"_a,
        "Returns True if any of the elements in the raster evaluate to True");

    mod.def("cluster_size",
        &pyalgo::clusterSize,
        "raster"_a,
        "include_diagonal"_a = false,
        "Create a raster containing the clusters, each cluster value is the size of the cluster");

    mod.def("cluster_id",
        &pyalgo::clusterId,
        "raster"_a,
        "include_diagonal"_a = false,
        "Create a raster containing the clusters, each cluster value is the id of the cluster");

    mod.def("cluster_id_with_obstacles",
        &pyalgo::clusterIdWithObstacles,
        "raster"_a,
        "obstacles"_a);

    mod.def("fuzzy_cluster_id",
        &pyalgo::fuzzyClusterId,
        "raster"_a,
        "radius"_a,
        "Equal items within radius distance (in meters) will get the same cluster id");

    mod.def("distance",
        &pyalgo::distance,
        "targets"_a,
        "Calculate the distance from a cell to the nearest target");

    mod.def("travel_distance",
        &pyalgo::travelDistance,
        "targets"_a,
        "traveltimes"_a,
        "Calculate the distance from a cell to the nearest target given the travel times");

    mod.def("sum_within_travel_distance",
        &pyalgo::sumWithinTravelDistance,
        "mask"_a,
        "resistance"_a,
        "values"_a,
        "max_resistance"_a,
        "include_adjacent"_a,
        "Calculate the sum of the cells within travel distance given the travel times");

    mod.def("closest_target",
        &pyalgo::closestTarget,
        "targets"_a,
        "Calculate the nearest target");

    mod.def("value_at_closest_target",
        &pyalgo::valueAtClosestTarget,
        "targets"_a,
        "values"_a,
        "Calculate the values at the nearest target");

    mod.def("value_at_closest_travel_target",
        &pyalgo::valueAtClosestTravelTarget,
        "targets"_a,
        "traveltimes"_a,
        "values"_a,
        "Calculate the values at the nearest target given the travel times");

    mod.def("value_at_closest_lt_travel_target",
        &pyalgo::valueAtClosestLessThenTravelTarget,
        "targets"_a,
        "traveltimes"_a,
        "max_traveltime"_a,
        "values"_a,
        "Calculate the values at the nearest target, nearer than max_traveltime, given the travel times");

    mod.def("csum",
        &pyalgo::categorySum,
        "clusters"_a,
        "values"_a,
        "Create a raster where the clusterids are replaced with the sum of the values in that cluster");

    mod.def("cmin",
        &pyalgo::categoryMin,
        "clusters"_a,
        "values"_a,
        "Create a raster where the clusterids are replaced with the minimum of the values in that cluster");

    mod.def("cmax",
        &pyalgo::categoryMax,
        "clusters"_a,
        "values"_a,
        "Create a raster where the clusterids are replaced with the maximum of the values in that cluster");

    mod.def("csum_in_buffer",
        &pyalgo::categorySumInBuffer,
        "clusters"_a,
        "values"_a,
        "radius"_a,
        "Create a raster where the clusterids are replaced with the sum of the values in that cluster in a given radius (in meter)");

    mod.def("filter_or",
        &pyalgo::categoryFilterOr,
        "clusters"_a,
        "filter"_a,
        "Create a raster where the clusterids are filtered based on the filter raster, if no values are true in a cluster the cluster becomes 0");

    mod.def("filter_and",
        &pyalgo::categoryFilterAnd,
        "clusters"_a,
        "filter"_a,
        "Create a raster where the clusterids are filtered based on the filter raster, if a value is false in a cluster the cluster becomes 0");

    mod.def("filter_not",
        &pyalgo::categoryFilterNot,
        "clusters"_a,
        "filter"_a,
        "Create a raster where the clusterids are filtered based on the filter raster, if a value is true in a cluster the cluster becomes 0");

    mod.def("reclass",
        py::overload_cast<const std::string&, py::object>(&pyalgo::reclass),
        "mapfile_path"_a,
        "raster1"_a,
        "Create a raster where the values are remapped based on the mapping in the map file");

    mod.def("reclass",
        py::overload_cast<const std::string&, py::object, py::object>(&pyalgo::reclass),
        "mapfile_path"_a,
        "raster1"_a,
        "raster2"_a,
        "Create a raster where the values are remapped based on the mapping in the map file");

    mod.def("reclass",
        py::overload_cast<const std::string&, py::object, py::object, py::object>(&pyalgo::reclass),
        "mapfile_path"_a,
        "raster1"_a,
        "raster2"_a,
        "raster3"_a,
        "Create a raster where the values are remapped based on the mapping in the map file");

    mod.def("reclassi",
        py::overload_cast<const std::string&, py::object, int>(&pyalgo::reclassi),
        "mapfile_path"_a,
        "raster"_a,
        "index"_a,
        "Create a raster where the values are remapped based on the mapping in the map file, the given column index in the mapping file is used (1-based)");

    mod.def("reclassi",
        py::overload_cast<const std::string&, py::object, py::object, int>(&pyalgo::reclassi),
        "mapfile_path"_a,
        "raster1"_a,
        "raster2"_a,
        "index"_a,
        "Create a raster where the values are remapped based on the mapping in the map file, the given column index in the mapping file is used (1-based)");

    mod.def("reclassi",
        py::overload_cast<const std::string&, py::object, py::object, py::object, int>(&pyalgo::reclassi),
        "mapfile_path"_a,
        "raster1"_a,
        "raster2"_a,
        "raster3"_a,
        "index"_a,
        "Create a raster where the values are remapped based on the mapping in the map file, the given column index in the mapping file is used (1-based)");

    mod.def("nreclass",
        &pyalgo::nreclass,
        "mapfile_path"_a,
        "raster"_a,
        "Create a raster where the values are remapped based on the ranged mapping in the map file");

    mod.def("sum_in_buffer",
        &pyalgo::sumInBuffer,
        "raster"_a,
        "radius"_a,
        "Resulting raster will have per cell the sum of all values within its radius.  Radius is in meters");

    mod.def("max_in_buffer",
        &pyalgo::maxInBuffer,
        "raster"_a,
        "radius"_a,
        "Resulting raster will have per cell the max of all values within its radius.  Radius is in meters");

    mod.def("raster_from_ndarray",
        &createFromNdArray,
        "array"_a,
        "metadata"_a,
        "Create a raster that contains the data from the python array."
        "The data will be copied, so modifying the python array will not change the raster");

    mod.def("blur_filter",
        &pyalgo::blurFilter,
        "raster"_a,
        "Apply a blur filter (average in 3x3 window) to the values in the raster");

    mod.def("majority_filter",
        &pyalgo::majorityFilter,
        "raster"_a,
        "radius"_a,
        "Apply a majority filter to the values in the raster for the given radius (in meter)");

    mod.def("max",
        &pyalgo::max,
        "Returns a raster where each value is the maximum value of the values in the raster arguments");

    mod.def("min",
        &pyalgo::min,
        "Returns a raster where each value is the maximum value of the values in the raster arguments");

    mod.def("clip",
        &pyalgo::clip,
        "raster"_a,
        "low"_a,
        "high"_a,
        "Returns a raster where each value is clipped between the low and high value");

    mod.def("clip_low",
        &pyalgo::clipLow,
        "raster"_a,
        "low"_a,
        "Returns a raster where each value lower than low is clipped to low");

    mod.def("clip_high",
        &pyalgo::clipHigh,
        "raster"_a,
        "high"_a,
        "Returns a raster where each value higher than high is clipped to high");

    mod.def("abs",
        &pyalgo::abs,
        "Returns a raster where each value is the absolute value of the provided raster");

    mod.def("round",
        &pyalgo::round,
        "Returns a raster where each value is the rounded value of the provided raster");

    mod.def("sin",
        &pyalgo::sin,
        "Returns a raster where each value is the sine value of the provided raster");

    mod.def("cos",
        &pyalgo::cos,
        "Returns a raster where each value is the cosine value of the provided raster");

    mod.def("log",
        &pyalgo::log,
        "Returns a raster where each value is the logarithm value of the provided raster");

    mod.def("log10",
        &pyalgo::log10,
        "Returns a raster where each value is the base 10 logarithm value of the provided raster");

    mod.def("exp",
        &pyalgo::exp,
        "Returns a raster where each value is the exponent value of the provided raster");

    mod.def("pow",
        &pyalgo::pow,
        "raster1"_a,
        "raster2"_a,
        "Returns a raster where each value is the value of raster1 to the power of the value of raster1");

    mod.def("raster_max",
        &pyalgo::maximumValue,
        "Returns the maximum value in the raster");

    mod.def("raster_min",
        &pyalgo::minimumValue,
        "raster"_a,
        "Returns the minimum value in the raster");

    mod.def("raster_sum",
        &pyalgo::sum,
        "Returns the sum of the values in the raster");

    mod.def("logical_and",
        py::overload_cast<py::object, py::object>(&pyalgo::logicalAnd),
        "raster1"_a,
        "raster2"_a,
        "Returns the logical and result of the two specfied rasters");

    mod.def("logical_and",
        py::overload_cast<py::object, py::object, py::object>(&pyalgo::logicalAnd),
        "raster1"_a,
        "raster2"_a,
        "raster3"_a,
        "Returns the logical and result of the three specfied rasters");

    mod.def("logical_or",
        py::overload_cast<py::object, py::object>(&pyalgo::logicalOr),
        "raster1"_a,
        "raster2"_a,
        "Returns the logical or result of the two specfied rasters.");

    mod.def("logical_or",
        py::overload_cast<py::object, py::object, py::object>(&pyalgo::logicalOr),
        "raster1"_a,
        "raster2"_a,
        "raster3"_a,
        "Returns the logical or result of the three specfied rasters.");

    mod.def("logical_not",
        &pyalgo::logicalNot,
        "Returns the logical or result of the specfied raster.");

    mod.def("is_nodata",
        &pyalgo::is_nodata,
        "raster"_a,
        "Returns a raster containing the value 1 for all the cells that are nodata in the provided raster");

    mod.def("rc_to_xy",
        &rowColCenterToXY,
        "metadata"_a,
        "cell"_a,
        "Returns the x-y coordinate of the centre of the row-col raster cell.");

    mod.def("xy_to_rc",
        &xYToRowCol,
        "metadata"_a,
        "point"_a,
        "Returns the row-col of the x-y coordinate.  The resulting row-col may be outside the raster, test this with the is_rc_on_raster function.");

    mod.def("is_rc_on_raster",
        &isRowColOnRaster,
        "metadata"_a,
        "cell"_a,
        "Test if the row-col is on the raster.");

    mod.def("normalise",
        &pyalgo::normalise,
        "raster"_a,
        "Returns a raster where the values are normalised between 0.0 and 1.0");

    mod.def("normalise_to_byte",
        &pyalgo::normaliseToByte,
        "raster"_a,
        "Returns a raster where the values are normalised between 0 and 255");

    mod.def("raster_equal",
        &pyalgo::rasterEqual,
        "Returns True if two arrays have the same shape and elements");

    mod.def("raster_stats",
        &pyalgo::statistics,
        "Returns a raster_stats data structure containing statistics of the provided raster");

    mod.def("table_row",
        &pyalgo::tableRow,
        "output"_a,
        "raster"_a,
        "category_raster"_a,
        "operation"_a,
        "label"_a,
        "append"_a = true,
        "Create a tab delimeted file that contains the operation result for each category in the raster");

    mod.def("raster_equal_one_of",
        &pyalgo::rasterEqualOneOf,
        "raster"_a,
        "values"_a,
        "Returns a raster with value 1 for the cells in raster that match one of the values in the values array");

    mod.def("allclose",
        &pyalgo::allClose,
        "rastera"_a,
        "rasterb"_a,
        "tolerance"_a = std::numeric_limits<double>::epsilon(),
        "Returns True if two arrays are element-wise equal within a tolerance");

    mod.def("isclose",
        &pyalgo::isClose,
        "rastera"_a,
        "rasterb"_a,
        "tolerance"_a = std::numeric_limits<double>::epsilon(),
        "Returns a boolean raster where two arrays are element-wise equal within a tolerance");

    mod.def("replace_value",
        &pyalgo::replaceValue,
        "raster"_a,
        "search_value"_a,
        "replace_value"_a,
        "Returns a raster where occurences of search_value in the raster are replaced by replace_value.");

    mod.def("replace_nodata",
        &pyalgo::replaceNodata,
        "raster"_a,
        "replace_value"_a,
        "Returns a raster where nodata values in the raster are replaced by replace_value. So the returned raster will not contain nodata values.");

    mod.def("draw_shape_on_raster",
        &pyalgo::drawShapeFileOnRaster,
        "raster"_a,
        "shapefile_path"_a,
        "Draws the shapes in the shapefile onto the supplied raster.");

    mod.def("if_then_else",
        &pyalgo::ifThenElse,
        "if_raster"_a,
        "then_raster"_a,
        "else_raster"_a,
        "Returns a raster that is a result of the evaluation of the provided raster."
        "All the nonzero values are replaced with the then_raster value, the other values are replaced with the else_raster values.");

    mod.def("warp_raster",
        &warp_raster,
        "raster"_a,
        "dest_epsg"_a,
        "Returns a raster that is the provided raster warped to the provided projection.");

    mod.def("resample",
        &resample,
        "raster"_a,
        "metadata"_a,
        "algoritm"_a = inf::gdal::ResampleAlgorithm::NearestNeighbour,
        "Returns a raster resampled to the provided metadata.");

    mod.def("create_map",
        py::overload_cast<py::object, py::object, bool>(&showRaster),
        "raster"_a,
        "color_map"_a = py::none(),
        "normalize"_a = true,
        "Create a folium Map instance that displays the raster over open street maps.");

    mod.def("create_map_from_array",
        py::overload_cast<py::array, const RasterMetadata&, py::object, bool>(&showRaster),
        "array"_a,
        "metadata"_a,
        "color_map"_a = py::none(),
        "normalize"_a = true,
        "Create a folium Map instance that displays the numpy array over open street maps.");

    mod.def("show_in_browser",
        &showRasterInBrowser,
        "raster"_a,
        "color_map"_a = py::none(),
        "normalize"_a = true,
        "Opens the system default browser to preview the provided raster.");

    mod.def("ldd_fix",
        &pyalgo::lddFix,
        "raster"_a,
        "Tries to fix an invalid ldd map. If it succeeds the fixed map is returned");

    mod.def("ldd_validate",
        &pyalgo::lddValidate,
        "ldd"_a,
        "loop_cb"_a           = py::none(),
        "invalid_value_cb"_a  = py::none(),
        "ends_in_nodata_cb"_a = py::none(),
        "outside_of_map_cb"_a = py::none(),
        "Returns true if the ldd is valid, callbacks can be provided for information about the detected errors");

    mod.def("accuflux",
        &pyalgo::accuflux,
        "ldd"_a,
        "freight"_a,
        "Accumulated material flowing into downstream cell");

    mod.def("accufractionflux",
        &pyalgo::accufractionflux,
        "ldd"_a,
        "freight"_a,
        "fraction"_a,
        "Fractional material transport downstream over local drain direction network");

    mod.def("flux_origin",
        &pyalgo::fluxOrigin,
        "ldd"_a,
        "freight"_a,
        "fraction"_a,
        "station"_a,
        "The upstream origin of the freight that accumulates in a station.  So if the freight at some cell C is 17, and 10% of it finally arrives in its downstream station, than the result map will contain at cell C the value 1.7");

    mod.def("ldd_cluster",
        &pyalgo::lddCluster,
        "ldd"_a,
        "id"_a,
        "All cells upstreams of a station will get the id of that station");

    mod.def("ldd_dist",
        &pyalgo::lddDist,
        "ldd"_a,
        "points"_a,
        "friction"_a,
        "Friction-distance from the cell under consideration to downstream nearest TRUE cell");

    mod.def("slopelength",
        &pyalgo::slopeLength,
        "ldd"_a,
        "friction"_a,
        "Accumulative-friction-distance of the longest accumulative-friction-path upstream over the local drain direction network cells against waterbasin divides");

    mod.def("max_upstream_dist",
        &pyalgo::max_upstream_dist,
        "ldd"_a,
        "Calculate the maximum upstream distance for each cell in the ldd");

    auto ioMod = mod.def_submodule("io");
    initIoModule(ioMod);
}
