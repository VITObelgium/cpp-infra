#include "infra/geometadata.h"
#include "infra/gdal.h"

#include <cmath>
#include <limits>
#include <sstream>

namespace inf {

GeoMetadata::GeoMetadata(int32_t _rows, int32_t _cols)
: GeoMetadata(_rows, _cols, 0.0, 0.0, 0.0, std::optional<double>())
{
}

GeoMetadata::GeoMetadata(int32_t _rows, int32_t _cols, std::optional<double> _nodatavalue)
: GeoMetadata(_rows, _cols, 0.0, 0.0, 0.0, _nodatavalue)
{
}

GeoMetadata::GeoMetadata(int32_t _rows, int32_t _cols, double _xll, double _yll, double _cellsize, std::optional<double> _nodatavalue)
: rows(_rows)
, cols(_cols)
, xll(_xll)
, yll(_yll)
, cellSize(_cellsize)
, nodata(_nodatavalue)
{
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
           std::fabs(cellSize - other.cellSize) < std::numeric_limits<double>::epsilon() &&
           nodataMatches &&
           projection == other.projection;
}

bool GeoMetadata::operator!=(const GeoMetadata& other) const noexcept
{
    return !(*this == other);
}

double GeoMetadata::convert_x_to_col_fraction(const double x) const
{
    return (x - xll) / cellSize;
}

double GeoMetadata::convert_y_to_row_fraction(const double y) const
{
    return rows - (y - yll) / cellSize;
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
    return (col + 0.5) * cellSize + xll;
}

double GeoMetadata::convert_row_centre_to_y(const int32_t row) const
{
    return (rows - row - 0.5) * cellSize + yll;
}

Point<double> GeoMetadata::convert_cell_centre_to_xy(const Cell& cell) const
{
    return Point<double>(convert_col_centre_to_x(cell.c), convert_row_centre_to_y(cell.r));
}

double GeoMetadata::convert_col_ll_to_x(const int32_t col) const
{
    return (col * cellSize) + xll;
}

double GeoMetadata::convert_row_ll_to_y(const int32_t row) const
{
    return (rows - 1 - row) * cellSize + yll;
}

Cell GeoMetadata::convert_xy_to_cell(const double x, const double y) const
{
    return Cell(convert_y_to_row(y), convert_x_to_col(x));
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

Point<double> GeoMetadata::center() const
{
    return Point<double>(xll + ((rows * cellSize) / 2), yll + ((rows * cellSize) / 2));
}

Point<double> GeoMetadata::top_left() const
{
    return Point<double>(xll, yll + (rows * cellSize));
}

Point<double> GeoMetadata::bottom_right() const
{
    return Point<double>(xll + (cols * cellSize), yll);
}

std::string GeoMetadata::to_string() const
{
    std::ostringstream os;
    os << "Rows: " << rows << " Cols: " << cols
       << " Xll: " << xll << " Yll: " << yll
       << " Cellsize: " << cellSize;

    if (nodata) {
        os << " NoData: " << *nodata;
    }

    if (!projection.empty()) {
        os << " Projection: " << projection;
    }

    return os.str();
}

std::optional<int32_t> GeoMetadata::projection_geo_epsg() const
{
    std::optional<int32_t> epsg;
    if (!projection.empty()) {
        epsg = inf::gdal::projection_to_geo_epsg(projection);
    }

    return epsg;
}

std::optional<int32_t> GeoMetadata::projection_epsg() const
{
    std::optional<int32_t> epsg;
    if (!projection.empty()) {
        try {
            epsg = inf::gdal::projection_to_epsg(projection);
        } catch (const std::exception&) {
        }
    }

    return epsg;
}

std::string GeoMetadata::projection_frienly_name() const
{
    return fmt::format("EPSG:{}", projection_epsg().value());
}

void GeoMetadata::set_projection_from_epsg(int32_t epsg)
{
    projection = inf::gdal::projection_from_epsg(epsg);
}

std::array<double, 6> metadata_to_geo_transform(const GeoMetadata& meta)
{
    return {{meta.xll, meta.cellSize, 0.0, meta.yll + (meta.cellSize * meta.rows), 0.0, -meta.cellSize}};
}

std::ostream& operator<<(std::ostream& os, const GeoMetadata& meta)
{
    return os << meta.to_string();
}
}
