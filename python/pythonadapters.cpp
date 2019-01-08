#include "pythonadapters.h"

#include "gdx/maskedrasterio.h"
#include "gdx/rasterspanio.h"
#include "infra/cast.h"
#include "infra/filesystem.h"

#include "gdx/algo/maximum.h"
#include "gdx/algo/normalise.h"
#include "gdx/algo/statistics.h"
#include "gdx/log.h"
#include "gdx/raster.h"
#include "gdx/rasterio.h"
#include "gdxconfig.h"
#include "pythonutils.h"
#include "rasterargument.h"

#include <cstdio>
#include <fmt/format.h>
#include <gsl/span>
#include <pybind11/eigen.h>
#include <sstream>

// Temporary cast fix to make array writeable until the problem is clear
namespace pybind11 {
template <typename T, detail::enable_if_t<!detail::is_pyobject<T>::value, int> = 0>
object cast(T& value, return_value_policy policy = return_value_policy::automatic_reference, handle parent = handle())
{
    if (policy == return_value_policy::automatic)
        policy = std::is_pointer<T>::value ? return_value_policy::take_ownership : return_value_policy::copy;
    else if (policy == return_value_policy::automatic_reference)
        policy = std::is_pointer<T>::value ? return_value_policy::reference : return_value_policy::copy;
    return reinterpret_steal<object>(detail::make_caster<T>::cast(value, policy, parent));
}
}

namespace gdx {

namespace py = pybind11;
using namespace py::literals;
using namespace inf;

const std::unordered_map<uint8_t, std::tuple<uint8_t, uint8_t, uint8_t>> cm::ldd{{{1, {50, 136, 189}},
    {2, {102, 194, 165}},
    {3, {171, 221, 164}},
    {4, {230, 245, 152}},
    {5, {255, 255, 191}},
    {6, {254, 224, 139}},
    {7, {253, 174, 97}},
    {8, {244, 109, 67}},
    {9, {213, 62, 79}}}};

template <typename StorageType>
static auto convertColorDescription(StorageType desc)
{
    std::vector<ColorDict::Entry> result;
    for (auto d : desc) {
        auto tup = d.template cast<py::tuple>();
        result.push_back({tup[0].template cast<float>(),
            tup[1].template cast<float>(),
            tup[2].template cast<float>()});
    }

    return result;
}

static ColorMap convertMatplotlibColorMapToColorMap(py::object colorMap)
{
    assert(colorMap);

    if (py::isinstance<py::dict>(colorMap)) {
        auto dict = colorMap.cast<py::dict>();

        std::array<Color, 256> cmap;
        for (size_t i = 0; i < cmap.size(); ++i) {
            if (dict.contains(py::int_(i))) {
                auto tup = dict[py::int_(i)].cast<py::tuple>();
                cmap[i]  = Color(tup[0].cast<uint8_t>(), tup[1].cast<uint8_t>(), tup[2].cast<uint8_t>());
            } else {
                cmap[i] = Color(0, 0, 0);
            }
        }

        return ColorMap(cmap);
    } else if (py::hasattr(colorMap, "_segmentdata")) {
        auto pydict = colorMap.attr("_segmentdata");

        if (pydict.contains("red")) {
            if (py::isinstance<py::tuple>(pydict["red"])) {
                ColorDict cdict;
                cdict.red   = convertColorDescription(pydict["red"].cast<py::tuple>());
                cdict.green = convertColorDescription(pydict["green"].cast<py::tuple>());
                cdict.blue  = convertColorDescription(pydict["blue"].cast<py::tuple>());
                return ColorMap(cdict);
            } else if (py::isinstance<py::list>(pydict["red"])) {
                ColorDict cdict;
                cdict.red   = convertColorDescription(pydict["red"].cast<py::list>());
                cdict.green = convertColorDescription(pydict["green"].cast<py::list>());
                cdict.blue  = convertColorDescription(pydict["blue"].cast<py::list>());
                return ColorMap(cdict);
            } else {
                std::array<Color, 256> cmap;
                for (size_t i = 0; i < cmap.size(); ++i) {
                    float pos = i / 255.f;
                    cmap[i]   = Color(static_cast<uint8_t>(pydict["red"](pos).cast<float>() * 255.f),
                        static_cast<uint8_t>(pydict["green"](pos).cast<float>() * 255.f),
                        static_cast<uint8_t>(pydict["blue"](pos).cast<float>() * 255.f));
                }
                return ColorMap(cmap);
            }
        } else {
            throw InvalidArgument("Unsupported color map");
        }
    } else if (py::hasattr(colorMap, "colors")) {
        auto colors = colorMap.attr("colors").cast<py::list>();
        std::vector<Color> clist;
        clist.reserve(colors.size());

        for (auto color : colors) {
            auto tup = color.cast<py::tuple>();
            clist.push_back(Color(static_cast<uint8_t>(std::round(tup[0].cast<float>() * 255.f)),
                static_cast<uint8_t>(std::round(tup[1].cast<float>() * 255.f)),
                static_cast<uint8_t>(std::round(tup[2].cast<float>() * 255.f))));
        }

        return ColorMap::qualitative(clist);
    } else {
        throw InvalidArgument("Unsupported color map");
    }
}

template <typename T>
using value_type = typename std::remove_cv_t<std::remove_reference_t<T>>::value_type;

std::tuple<double, double> rowColCenterToXY(const gdx::RasterMetadata& meta, const std::tuple<int32_t, int32_t>& cell)
{
    auto height        = meta.rows * meta.cellSize;
    auto minX          = meta.xll;
    auto minY          = meta.yll;
    auto maxY          = minY + height;
    auto topLeftCorner = Point<double>(minX, maxY);

    auto point = gdx::Point<double>(topLeftCorner.x + ((std::get<1>(cell) + 0.5) * meta.cellSize) /*col*/,
        topLeftCorner.y - ((std::get<0>(cell) + 0.5) * meta.cellSize) /*row*/);
    return std::make_tuple(point.x, point.y);
}

std::tuple<int32_t, int32_t> xYToRowCol(const gdx::RasterMetadata& meta, const std::tuple<double, double>& point)
{
    auto yTopLeft = meta.yll + meta.rows * meta.cellSize;
    auto x        = std::get<0>(point);
    auto y        = std::get<1>(point);
    int32_t col   = int32_t((x - meta.xll) / meta.cellSize);
    int32_t row   = int32_t((yTopLeft - y) / meta.cellSize);
    return std::make_tuple(row, col);
}

bool isRowColOnRaster(const gdx::RasterMetadata& meta, const std::tuple<int32_t, int32_t>& cell)
{
    auto row = std::get<0>(cell);
    auto col = std::get<1>(cell);
    return 0 <= row && row < meta.rows && 0 <= col && col < meta.cols;
}

py::list rasterBounds(const RasterMetadata& meta, bool projected)
{
    auto width  = meta.cols * meta.cellSize;
    auto height = meta.rows * meta.cellSize;

    auto minX = meta.xll;
    auto maxX = minX + width;
    auto minY = meta.yll;
    auto maxY = minY + height;

    auto topLeftCorner     = Point<double>(minX, maxY);
    auto bottomRightCorner = Point<double>(maxX, minY);

    if (meta.projection.empty()) {
        topLeftCorner     = Point<double>(0.0, meta.rows / meta.cellSize);
        bottomRightCorner = Point<double>(meta.cols / meta.cellSize, 0.0);
    } else if (!projected) {
        auto sourceEpsg = meta.projection_epsg();
        if (!sourceEpsg.has_value()) {
            throw RuntimeError("Metadata does not contain projection information");
        }
        topLeftCorner     = io::gdal::projected_to_geographic(sourceEpsg.value(), topLeftCorner);
        bottomRightCorner = io::gdal::projected_to_geographic(sourceEpsg.value(), bottomRightCorner);
    }

    py::list topleftArray(2);
    topleftArray[0] = topLeftCorner.y;
    topleftArray[1] = topLeftCorner.x;

    py::list bottomRightArray(2);
    bottomRightArray[0] = bottomRightCorner.y;
    bottomRightArray[1] = bottomRightCorner.x;

    py::list bounds(2);
    bounds[0] = topleftArray;
    bounds[1] = bottomRightArray;
    return bounds;
}

template <typename T>
std::pair<py::array, py::object> createDataMaskPair(Raster& raster)
{
    py::object parent = py::cast(raster);
    py::object mask;

    auto& mask_data = raster.mask_data<T>();
    if (mask_data.size() > 0) {
        mask = py::cast(mask_data, py::return_value_policy::reference_internal, parent);
    }

    return {py::cast(raster.eigen_data<T>(), py::return_value_policy::reference_internal, parent), mask};
}

static std::pair<py::array, py::object> rasterNumpyArray(Raster& raster)
{
    auto& type = raster.type();
    if (type == typeid(uint8_t)) return createDataMaskPair<uint8_t>(raster);
    if (type == typeid(int16_t)) return createDataMaskPair<int16_t>(raster);
    if (type == typeid(uint16_t)) return createDataMaskPair<uint16_t>(raster);
    if (type == typeid(int32_t)) return createDataMaskPair<int32_t>(raster);
    if (type == typeid(uint32_t)) return createDataMaskPair<uint32_t>(raster);
    if (type == typeid(float)) return createDataMaskPair<float>(raster);
    if (type == typeid(double)) return createDataMaskPair<double>(raster);

    throw InvalidArgument("Invalid raster data type");
}

py::object rasterNumpyMaskedArray(Raster& raster)
{
    py::object maskedArray;
    auto ma           = py::module::import("numpy.ma");
    auto [data, mask] = rasterNumpyArray(raster);
    if (mask) {
        maskedArray = ma.attr("array")(data, "mask"_a = mask, "fill_value"_a = raster.metadata().nodata.value_or(0));
    } else {
        maskedArray = ma.attr("array")(data);
    }

    maskedArray.attr("_sharedmask") = false; // makes sure changes to the mask in python are done in the raster mask data
    return maskedArray;
}

std::string rasterRepresentation(const gdx::Raster& raster)
{
    auto& meta = raster.metadata();
    auto type  = py::str(rasterTypeToDtype(raster.type()));
    return fmt::format("<gdx.raster {}x{} dtype={}>", meta.rows, meta.cols, static_cast<std::string>(type));
}

Raster castRaster(const Raster& raster, pybind11::object dataType)
{
    return raster.cast(dtypeToRasterType(py::dtype::from_args(dataType)));
}

Raster read_raster(py::object fileName)
{
    return Raster::read(handle_path(fileName));
}

Raster read_raster(py::object fileName, const RasterMetadata& extent)
{
    return Raster::read(handle_path(fileName), extent);
}

Raster read_raster(py::object dataType, py::object fileName)
{
    return Raster::read(handle_path(fileName), dtypeToRasterType(py::dtype::from_args(dataType)));
}

void write_raster(Raster& raster, const std::string& filepath, py::object colorMap)
{
    if (colorMap.is_none()) {
        raster.write(fs::u8path(filepath));
    } else {
        raster.writeColorMapped(filepath, convertMatplotlibColorMapToColorMap(colorMap));
    }
}

void write_raster(py::object dataType, Raster& raster, const std::string& filepath, py::object colorMap)
{
    auto& dtype = dtypeToRasterType(py::dtype::from_args(dataType));

    if (colorMap.is_none()) {
        raster.write(fs::u8path(filepath), dtype);
    } else {
        throw RuntimeError("Color maps with custom types currently not supported");
        //raster.writeColorMapped(filePath, dtype, convertMatplotlibColorMapToColorMap(colorMap));
    }
}

template <typename T>
Raster createFromNdArray(py::array_t<T> arrayData, const gdx::RasterMetadata& meta)
{
    auto dataSpan = gsl::span<const T>(reinterpret_cast<const T*>(arrayData.data()), arrayData.size());
    return Raster(MaskedRaster<T>(meta, dataSpan));
}

Raster createFromNdArray(py::array arrayData, const gdx::RasterMetadata& meta)
{
    if (arrayData.ndim() != 2) {
        throw InvalidArgument("Only 2 dimenstional numpy arrays are supported as data source");
    }

    auto rows = static_cast<int32_t>(arrayData.shape(0));
    auto cols = static_cast<int32_t>(arrayData.shape(1));

    if (rows != meta.rows || cols != meta.cols) {
        throw RuntimeError("Numpy array shape does not match provided metadata: {}x{} <-> {}x{}", rows, cols, meta.rows, meta.cols);
    }

    // undo the type erasure
    auto& type = dtypeToRasterType(arrayData.dtype());
    if (type == typeid(bool)) return createFromNdArray<uint8_t>(arrayData, meta);
    if (type == typeid(uint8_t)) return createFromNdArray<uint8_t>(arrayData, meta);
    if (type == typeid(int16_t)) return createFromNdArray<int16_t>(arrayData, meta);
    if (type == typeid(uint16_t)) return createFromNdArray<uint16_t>(arrayData, meta);
    if (type == typeid(int32_t)) return createFromNdArray<int32_t>(arrayData, meta);
    if (type == typeid(uint32_t)) return createFromNdArray<uint32_t>(arrayData, meta);
    if (type == typeid(float)) return createFromNdArray<float>(arrayData, meta);
    if (type == typeid(double)) return createFromNdArray<double>(arrayData, meta);

    throw InvalidArgument("Invalid raster data type");
}

pybind11::str showMetadata(const RasterMetadata& meta)
{
    std::stringstream ss;

    auto row = [](std::ostream& os, auto&& name, auto&& value) {
        os << "<tr><td style=\"text-align:center;\">" << name << "</td><td style=\"text-align:center;\">" << value << "</td></tr>";
    };

    ss << "<table>";
    row(ss, "Dimensions", fmt::format("{}x{}", meta.cols, meta.rows));
    row(ss, "Cell size", meta.cellSize);
    row(ss, "Xll", meta.xll);
    row(ss, "Yll", meta.yll);

    if (meta.nodata.has_value()) {
        row(ss, "NoData", *meta.nodata);
    }

    if (!meta.projection.empty()) {
        try {
            row(ss, "Projection", meta.projection_frienly_name());
        } catch (const std::exception&) {
            row(ss, "Projection", meta.projection);
        }
    }

    ss << "</table>";
    return ss.str();
}

pybind11::str showRasterStats(const RasterStats<512>& stats)
{
    std::stringstream ss;

    auto row = [](std::ostream& os, auto&& name, auto&& value) {
        os << "<tr><td style=\"text-align:center;\">" << name << "</td><td style=\"text-align:center;\">" << value << "</td></tr>";
    };

    ss << "<table>";
    row(ss, "Minimum", stats.lowestValue);
    row(ss, "Maximum", stats.highestValue);
    row(ss, "Nodata values", stats.noDataValues);
    row(ss, "Zero values", stats.zeroValues);
    row(ss, "NaN values", stats.nanValues);
    row(ss, "Negative values", stats.negativeValues);
    row(ss, "Sigma non zero", stats.sigmaNonZero);
    row(ss, "Sum", stats.sum);
    ss << "</table>";

    return ss.str();
}

static std::string createTemporaryFilePath(const std::string& filename, const std::string& extension)
{
#ifdef INFRA_HAS_FILESYSTEM
    static int count = 0;
    auto tempDir     = fs::temp_directory_path();
    return (tempDir / fmt::format("{}{}.{}", filename, ++count, extension)).string();
#else
#ifdef WIN32
    return "C:/Temp/gdxraster.png";
#else
    return "/tmp/gdxraster.png";
#endif
#endif
}

template <typename RasterType>
void writeAsPng(const RasterType& ras, const RasterMetadata& meta, const fs::path& filename, py::object colorMap, bool normalize)
{
    using T = typename RasterType::value_type;

    // remap to byte and normalize the data
    if (colorMap.is_none()) {
        if (normalize) {
            auto normMeta = meta;
            if (normMeta.nodata.has_value()) {
                normMeta.nodata = std::numeric_limits<uint8_t>::max();
            }
            MaskedRaster<uint8_t> normalisedData(normMeta);
            normalise(ras, normalisedData);
            gdx::write_raster(normalisedData, filename);
        } else if constexpr (!(std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>)) {
            std::vector<uint8_t> clippedData;
            auto clippedMeta = io::cast_raster<T, uint8_t>(meta, ras, clippedData);
            gdx::io::write_raster(clippedData, clippedMeta, filename);
        } else {
            gdx::write_raster(ras, filename);
        }
    } else {
        auto cmap = convertMatplotlibColorMapToColorMap(colorMap);
        if (normalize) {
            auto normMeta = meta;
            if (normMeta.nodata.has_value()) {
                normMeta.nodata = std::numeric_limits<uint8_t>::max();
            }
            MaskedRaster<uint8_t> normalisedData(normMeta);
            normalise(ras, normalisedData);
            gdx::write_rasterColorMapped(normalisedData, filename, cmap);
        } else if constexpr (!(std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>)) {
            std::vector<uint8_t> clippedData(ras.size());
            auto clippedMeta = io::cast_raster<T, uint8_t>(meta, ras, clippedData);
            gdx::io::write_raster_color_mapped(clippedData, clippedMeta, filename, cmap);
        } else {
            gdx::write_rasterColorMapped<T>(ras, filename, cmap);
        }
    }
}

static py::object createFoliumColorMap(py::object colorMap, double minValue, double maxValue)
{
    if (std::isnan(minValue) || std::isnan(maxValue)) {
        return py::none();
    }

    auto cm = py::module::import("branca.colormap");
    py::list cmapData;

    if (colorMap.is_none()) {
        for (int i = 0; i < 255; ++i) {
            cmapData.append(py::make_tuple(uint8_t(i), uint8_t(i), uint8_t(i)));
        }
    } else {
        auto cmap = convertMatplotlibColorMapToColorMap(colorMap);
        for (int i = 0; i < 255; ++i) {
            auto color = cmap.get_color(uint8_t(i));
            cmapData.append(py::make_tuple(color.r, color.g, color.b));
        }
    }

    return cm.attr("LinearColormap")("colors"_a = cmapData, "vmin"_a = minValue, "vmax"_a = maxValue);
}

template <typename RasterType>
py::object createFoliumMap(const RasterType& data, const RasterMetadata& meta, py::object colorMap, bool normalize)
{
    bool hasProjectionInfo = !meta.projection.empty();
    auto tmpPath           = createTemporaryFilePath("gdxraster", "png");
    auto bounds            = rasterBounds(meta, !hasProjectionInfo);

    writeAsPng(data, meta, tmpPath, colorMap, normalize);

    auto folium             = py::module::import("folium");
    auto foliumRasterLayers = py::module::import("folium.raster_layers");
    auto builtins           = py::module::import("builtins");

    auto opacity     = hasProjectionInfo ? 0.8 : 1.0;
    py::str crs      = hasProjectionInfo ? "EPSG3857"_s : "Simple"_s;
    py::object tiles = py::none();
    if (hasProjectionInfo) {
        tiles = "OpenStreetMap"_s;
    }
    auto map     = folium.attr("Map")("crs"_a = crs, "tiles"_a = tiles, "attr"_a = "VITO");
    auto overlay = foliumRasterLayers.attr("ImageOverlay")(tmpPath, "opacity"_a = opacity, "bounds"_a = bounds, "pixelated"_a = true);

    double minValue = 0.0;
    double maxValue = 255.0;

    map.attr("add_child")(overlay);
    map.attr("fit_bounds")(bounds);

    if (normalize) {
        try {
            std::tie(minValue, maxValue) = gdx::minmax(data);
        } catch (const InvalidArgument&) {
            // raster contains only nodata
            Log::warn("Raster contains only nodata values");
        }
    }

    auto cm = createFoliumColorMap(colorMap, minValue, maxValue);
    if (!cm.is_none()) {
        map.attr("add_child")(cm);
    }

    return map;
}

template <typename T>
py::object showRaster(raster_span<const T> data, py::object colorMap, bool normalize)
{
    std::unique_ptr<std::vector<T>> workRaster;
    auto displayData       = data;
    auto displayMeta       = data.metadata();
    bool hasProjectionInfo = !displayMeta.projection.empty();
    if (hasProjectionInfo) {
        displayMeta = gdal::warp_metadata(displayMeta, 3857);
        workRaster  = std::make_unique<std::vector<T>>(displayMeta.rows * displayMeta.cols);
        gdx::io::warp_raster<T, T>(data, data.metadata(), *workRaster, displayMeta);
        displayData = raster_span<const T>(*workRaster, displayMeta);
    }

    return createFoliumMap(displayData, displayMeta, colorMap, normalize);
}

py::object showRaster(py::object rasterArg, py::object colorMap, bool normalize)
{
    return std::visit([&](auto&& raster) {
        using RasterT = std::remove_cv_t<std::remove_reference_t<decltype(raster)>>;

        std::unique_ptr<RasterT> workRaster;

        auto* displayRaster    = &raster;
        py::str tiles          = py::none();
        bool hasProjectionInfo = !raster.metadata().projection.empty();
        if (hasProjectionInfo) {
            workRaster    = std::make_unique<RasterT>(gdx::warp_raster(raster, 3857));
            displayRaster = workRaster.get();
        }

        return createFoliumMap(*displayRaster, displayRaster->metadata(), colorMap, normalize);
    },
        RasterArgument(rasterArg).variant());
}

pybind11::object showRaster(pybind11::array arr, const RasterMetadata& meta, pybind11::object colorMap, bool normalize)
{
    return visitArray(arr, [&](auto&& typedArray) {
        using T = typename std::decay_t<decltype(typedArray)>::value_type;
        return showRaster<T>(make_raster_span<const T>(static_cast<const T*>(arr.data()), meta), colorMap, normalize);
    });
}

void showRasterInBrowser(pybind11::object rasterArg, pybind11::object colorMap, bool normalize)
{
    auto foliumMap = showRaster(rasterArg, colorMap, normalize);

    auto webBrowser = py::module::import("webbrowser");
    auto filename   = createTemporaryFilePath("index", "html");

    // Write the map html to a temporary file and open a browser
    foliumMap.attr("save")(filename);
    webBrowser.attr("open_new")(py::str("file://" + filename));
}

Raster warp_raster(py::object rasterArg, int32_t epsg)
{
    return std::visit([&](auto&& raster) {
        return Raster(gdx::warp_raster(raster, epsg));
    },
        RasterArgument(rasterArg).variant());
}

Raster resample(py::object rasterArg, const RasterMetadata& meta, inf::gdal::ResampleAlgorithm algo)
{
    return std::visit([&](auto&& raster) {
        using T = typename std::decay_t<decltype(raster)>::value_type;
        return Raster(gdx::resample_raster<T>(raster, meta, algo));
    },
        RasterArgument(rasterArg).variant());
}
}
