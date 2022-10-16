#include "infra/geometadata.h"

#ifdef INFRA_GDAL_ENABLED
#include "infra/gdal.h"
#endif

#include <cmath>
#include <limits>
#include <sstream>

namespace inf {

GeoMetadata::GeoMetadata(int32_t rows_, int32_t cols_)
: GeoMetadata(rows_, cols_, 0.0, 0.0, 0.0, std::optional<double>())
{
}

GeoMetadata::GeoMetadata(int32_t rows_, int32_t cols_, std::optional<double> nodatavalue_)
: GeoMetadata(rows_, cols_, 0.0, 0.0, 0.0, nodatavalue_)
{
}

GeoMetadata::GeoMetadata(int32_t rows_, int32_t cols_, double xll_, double yll_, double cellsize_, std::optional<double> nodatavalue_)
: GeoMetadata(rows_, cols_, xll_, yll_, CellSize(cellsize_, -cellsize_), nodatavalue_)
{
}

GeoMetadata::GeoMetadata(int32_t rows_, int32_t cols_, double xll_, double yll_, double cellsize_, std::optional<double> nodatavalue_, std::string_view projection_)
: GeoMetadata(rows_, cols_, xll_, yll_, CellSize(cellsize_, -cellsize_), nodatavalue_, projection_)
{
}

GeoMetadata::GeoMetadata(int32_t rows_, int32_t cols_, double xll_, double yll_, CellSize cellsize_, std::optional<double> nodatavalue_)
: GeoMetadata(rows_, cols_, xll_, yll_, cellsize_, nodatavalue_, {})
{
}

GeoMetadata::GeoMetadata(int32_t rows_, int32_t cols_, double xll_, double yll_, CellSize cellsize_, std::optional<double> nodatavalue_, std::string_view projection_)
: rows(rows_)
, cols(cols_)
, xll(xll_)
, yll(yll_)
, cellSize(cellsize_)
, nodata(nodatavalue_)
, projection(projection_)
{
}

bool GeoMetadata::is_north_up() const noexcept
{
    return cellSize.y < 0;
}

void GeoMetadata::set_cell_size(double size) noexcept
{
    cellSize.x = size;
    cellSize.y = -size;
}

double GeoMetadata::cell_size_x() const noexcept
{
    return cellSize.x;
}

double GeoMetadata::cell_size_y() const noexcept
{
    return cellSize.y;
}

bool GeoMetadata::operator==(const GeoMetadata& other) const noexcept
{
    bool nodataMatches = false;
    if (nodata && other.nodata) {
        if (std::isnan(*nodata) && std::isnan(*other.nodata)) {
            nodataMatches = true;
        } else {
            nodataMatches = nodata == other.nodata;
        }
    } else {
        nodataMatches = nodata == other.nodata;
    }

    return rows == other.rows &&
           cols == other.cols &&
           std::fabs(xll - other.xll) < std::numeric_limits<double>::epsilon() &&
           std::fabs(yll - other.yll) < std::numeric_limits<double>::epsilon() &&
           cellSize == other.cellSize &&
           nodataMatches &&
           projected_epsg() == other.projected_epsg();
}

bool GeoMetadata::operator!=(const GeoMetadata& other) const noexcept
{
    return !(*this == other);
}

double GeoMetadata::convert_x_to_col_fraction(double x) const
{
    return (x - xll) / cellSize.x;
}

double GeoMetadata::convert_y_to_row_fraction(double y) const
{
    return (y - top_left().y) / cellSize.y;
}

int32_t GeoMetadata::convert_x_to_col(const double x) const
{
    return int32_t(std::floor(convert_x_to_col_fraction(x)));
}

int32_t GeoMetadata::convert_y_to_row(const double y) const
{
    return int32_t(std::floor(convert_y_to_row_fraction(y)));
}

double GeoMetadata::convert_col_centre_to_x(const int32_t col) const
{
    return xll + ((col + 0.5) * cellSize.x);
}

double GeoMetadata::convert_row_centre_to_y(const int32_t row) const
{
    return yll - ((rows - row - 0.5) * cellSize.y);
}

Point<double> GeoMetadata::convert_cell_centre_to_xy(const Cell& cell) const
{
    return Point<double>(convert_col_centre_to_x(cell.c), convert_row_centre_to_y(cell.r));
}

double GeoMetadata::convert_col_ll_to_x(const int32_t col) const
{
    return xll + (col * cellSize.x);
}

double GeoMetadata::convert_row_ll_to_y(const int32_t row) const
{
    return yll - ((rows - 1 - row) * cellSize.y);
}

Point<double> GeoMetadata::convert_cell_ll_to_xy(const Cell& cell) const
{
    return Point<double>(convert_col_ll_to_x(cell.c), convert_row_ll_to_y(cell.r));
}

Cell GeoMetadata::convert_xy_to_cell(const double x, const double y) const
{
    return Cell(convert_y_to_row(y), convert_x_to_col(x));
}

Cell GeoMetadata::convert_coordinate_to_cell(const Coordinate& coord) const
{
    return Cell(convert_y_to_row(coord.latitude), convert_x_to_col(coord.longitude));
}

bool GeoMetadata::is_xy(const double x, const double y, const int32_t row, const int32_t col) const
{
    return (row == convert_y_to_row(y) && col == convert_x_to_col(x));
}

bool GeoMetadata::is_on_map(const Cell& cell) const
{
    return is_on_map(cell.r, cell.c);
}

bool GeoMetadata::is_on_map(const int32_t r, const int32_t c) const
{
    return r < rows && c < cols && r >= 0 && c >= 0;
}

void GeoMetadata::compute_rect_on_map_around(const int32_t row, const int32_t col, const int32_t radius, int32_t& r0, int32_t& c0, int32_t& r1, int32_t& c1) const
{
    r0 = row - radius;
    if (r0 < 0) {
        r0 = 0;
    }
    r1 = row + radius;
    if (r1 > rows - 1) {
        r1 = rows - 1;
    }

    c0 = col - radius;
    if (c0 < 0) {
        c0 = 0;
    }
    c1 = col + radius;
    if (c1 > cols - 1) {
        c1 = cols - 1;
    }
}

Rect<double> GeoMetadata::bounding_box() const noexcept
{
    Rect<double> result;
    auto width         = cellSize.x * cols;
    result.topLeft     = Point<double>(xll, yll - (cellSize.y * rows));
    result.bottomRight = Point<double>(result.topLeft.x + width, yll);
    return result;
}

Rect<double> GeoMetadata::bounding_box(const Cell& cell) const noexcept
{
    Point<double> ll = convert_cell_ll_to_xy(cell);

    Rect<double> result;
    result.topLeft     = Point(ll.x, ll.y - cellSize.y);
    result.bottomRight = Point(result.topLeft.x + cellSize.x, ll.y);
    return result;
}

Point<double> GeoMetadata::center() const
{
    return Point<double>(xll + ((cols * cellSize.x) / 2), yll - ((rows * cellSize.y) / 2));
}

Point<double> GeoMetadata::top_left() const
{
    return Point<double>(xll, yll - (rows * cellSize.y));
}

Point<double> GeoMetadata::top_left_center() const
{
    return Point<double>(xll + (cellSize.x / 2.0), yll - (rows * cellSize.y) + (cellSize.y / 2.0));
}

Point<double> GeoMetadata::bottom_right() const
{
    return Point<double>(xll + (cols * cellSize.x), yll);
}

std::string GeoMetadata::to_string() const
{
    std::ostringstream os;
    os << fmt::format("Rows: {} Cols: {} Xll: {:.3f} Yll: {:.3f} Cellsize: {},{}", rows, cols, xll, yll, cellSize.x, cellSize.y);

    if (nodata) {
        os << " NoData: " << *nodata;
    }

    if (!projection.empty()) {
        os << " Projection: " << projection;
    }

    return os.str();
}

double GeoMetadata::width() const noexcept
{
    return cols * cellSize.x;
}

double GeoMetadata::height() const noexcept
{
    return rows * std::abs(cellSize.y);
}

std::optional<int32_t> GeoMetadata::projection_geo_epsg() const noexcept
{
    return geographic_epsg();
}

std::optional<int32_t> GeoMetadata::geographic_epsg() const noexcept
{
    std::optional<int32_t> epsg;

#ifdef INFRA_GDAL_ENABLED
    if (!projection.empty()) {
        epsg = inf::gdal::projection_to_geo_epsg(projection);
    }
#endif

    return epsg;
}

std::optional<int32_t> GeoMetadata::projection_epsg() const noexcept
{
    return projected_epsg();
}

std::optional<int32_t> GeoMetadata::projected_epsg() const noexcept
{
    std::optional<int32_t> epsg;

#ifdef INFRA_GDAL_ENABLED
    if (!projection.empty()) {
        epsg = inf::gdal::projection_to_epsg(projection);
    }
#endif

    return epsg;
}

std::string GeoMetadata::projection_frienly_name() const noexcept
{
    if (auto epsg = projected_epsg(); epsg.has_value()) {
        return fmt::format("EPSG:{}", *epsg);
    }

    return std::string();
}

void GeoMetadata::set_projection_from_epsg(int32_t epsg)
{
#ifdef INFRA_GDAL_ENABLED
    projection = gdal::projection_from_epsg(epsg);
#else
    projection = fmt::format("EPSG:{}", epsg);
#endif
}

std::array<double, 6> metadata_to_geo_transform(const GeoMetadata& meta)
{
    return {{meta.xll, meta.cellSize.x, 0.0, meta.yll - (meta.cellSize.y * meta.rows), 0.0, meta.cellSize.y}};
}

std::ostream& operator<<(std::ostream& os, const GeoMetadata& meta)
{
    return os << meta.to_string();
}

GeoMetadata copy_metadata_replace_nodata(const GeoMetadata& meta, std::optional<double> nodata)
{
    GeoMetadata result = meta;
    result.nodata      = nodata;
    return result;
}

static Rect<double> metadata_intersion_rectangle(const GeoMetadata& meta1, const GeoMetadata& meta2)
{
    if (meta1.projection != meta2.projection) {
        throw RuntimeError("Cannot intersect metadata with different projections");
    }

    if (meta1.cellSize.x != meta2.cellSize.x || meta1.cellSize.y != meta2.cellSize.y) {
        if (!metadata_is_aligned(meta1, meta2)) {
            throw InvalidArgument("Extents cellsize does not match {} <-> {}", meta1.cellSize, meta2.cellSize);
        }
    }

    if (meta1.cellSize.x == 0) {
        throw InvalidArgument("Extents cellsize is zero");
    }

    return rectangle_intersection(meta1.bounding_box(), meta2.bounding_box());
}

bool metadata_intersects(const GeoMetadata& meta1, const GeoMetadata& meta2)
{
    const auto intersection = metadata_intersion_rectangle(meta1, meta2);
    return intersection.is_valid() && intersection.width() > 0 && intersection.height() > 0;
}

GeoMetadata metadata_intersection(const GeoMetadata& meta1, const GeoMetadata& meta2)
{
    auto intersection = rectangle_intersection(meta1.bounding_box(), meta2.bounding_box());
    GeoMetadata result;
    result.projection = meta1.projection;
    result.nodata     = meta1.nodata;
    if (!result.nodata.has_value()) {
        result.nodata = meta2.nodata;
    }

    if (intersection.is_valid() && intersection.width() > 0 && intersection.height() > 0) {
        result.xll      = intersection.bottom_left().x;
        result.yll      = intersection.bottom_left().y;
        result.cellSize = meta1.cellSize;

        result.rows = truncate<int32_t>(std::round(intersection.height() / std::abs(result.cell_size_y())));
        result.cols = truncate<int32_t>(std::round(intersection.width() / std::abs(result.cell_size_x())));
    }

    return result;
}

static bool is_aligned(double val1, double val2, double cellsize)
{
    auto diff = std::abs(val1 - val2);
    return std::remainder(diff, cellsize) < 1e-12;
}

bool metadata_is_aligned(const GeoMetadata& meta1, const GeoMetadata& meta2) noexcept
{
    auto cellSizeX1 = meta1.cell_size_x();
    auto cellSizeX2 = meta2.cell_size_x();

    auto cellSizeY1 = std::abs(meta1.cell_size_y());
    auto cellSizeY2 = std::abs(meta2.cell_size_y());

    if (cellSizeX1 != cellSizeX2) {
        if (cellSizeX1 < cellSizeX2) {
            std::swap(cellSizeX1, cellSizeX2);
        }

        if (std::fmod(cellSizeX1, cellSizeX2) != 0) {
            return false;
        }
    }

    if (cellSizeY1 != cellSizeY2) {
        if (cellSizeY1 < cellSizeY2) {
            std::swap(cellSizeY1, cellSizeY2);
        }

        if (std::fmod(cellSizeY1, cellSizeY2) != 0) {
            return false;
        }
    }

    return is_aligned(meta1.xll, meta2.xll, meta1.cell_size_x()) &&
           is_aligned(meta1.yll, meta2.yll, std::abs(meta1.cell_size_y()));
}
}
