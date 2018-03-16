#include "infra/geometadata.h"

#include <cmath>
#include <limits>
#include <sstream>

namespace infra {

GeoMetadata::GeoMetadata(int32_t _rows, int32_t _cols)
: GeoMetadata(_rows, _cols, 0.0, 0.0, 0.0, std::optional<double>())
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

double GeoMetadata::convertXtoColFraction(const double x) const
{
    return (x - xll) / cellSize;
}

double GeoMetadata::convertYtoRowFraction(const double y) const
{
    return rows - (y - yll) / cellSize;
}

int32_t GeoMetadata::convertXtoCol(const double x) const
{
    return int32_t(std::floor(convertXtoColFraction(x)));
}

int32_t GeoMetadata::convertYtoRow(const double y) const
{
    return int32_t(std::floor(convertYtoRowFraction(y)));
}

double GeoMetadata::convertColCentreToX(const int32_t col) const
{
    return (col + 0.5) * cellSize + xll;
}

double GeoMetadata::convertRowCentreToY(const int32_t row) const
{
    return (rows - row - 0.5) * cellSize + yll;
}

double GeoMetadata::convertColLLToX(const int32_t col) const
{
    return (col * cellSize) + xll;
}

double GeoMetadata::convertRowLLToY(const int32_t row) const
{
    return (rows - 1 - row) * cellSize + yll;
}

Cell<int32_t> GeoMetadata::convertXYtoCell(const double x, const double y) const
{
    return Cell<int32_t>(convertYtoRow(y), convertXtoCol(x));
}

bool GeoMetadata::isXY(const double x, const double y, const int32_t row, const int32_t col) const
{
    return (row == convertYtoRow(y) && col == convertXtoCol(x));
}

bool GeoMetadata::isOnMap(const Cell<int32_t>& cell) const
{
    return isOnMap(cell.r, cell.c);
}

bool GeoMetadata::isOnMap(const int32_t r, const int32_t c) const
{
    return r < rows && c < cols && r >= 0 && c >= 0;
}

void GeoMetadata::computeRectOnMapAround(const int32_t row, const int32_t col, const int32_t radius, int32_t& r0, int32_t& c0, int32_t& r1, int32_t& c1) const
{
    r0 = row - radius;
    r1 = row + radius;
    if (r1 > rows - 1) {
        r1 = rows - 1;
    }

    c0 = col - radius;
    c1 = col + radius;
    if (c1 > cols - 1) {
        c1 = cols - 1;
    }
}

Point<double> GeoMetadata::center() const
{
    return Point<double>(xll + ((rows * cellSize) / 2), yll + ((rows * cellSize) / 2));
}

Point<double> GeoMetadata::topLeft() const
{
    return Point<double>(xll, yll + (rows * cellSize));
}

Point<double> GeoMetadata::bottomRight() const
{
    return Point<double>(xll + (cols * cellSize), yll);
}

std::string GeoMetadata::toString() const
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

std::array<double, 6> metadataToGeoTransform(const GeoMetadata& meta)
{
    return {{meta.xll, meta.cellSize, 0.0, meta.yll + (meta.cellSize * meta.rows), 0.0, -meta.cellSize}};
}
}
