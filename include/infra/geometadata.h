#pragma once

#include "infra/cell.h"

#include <optional>
#include <sstream>

namespace infra {

struct GeoMetadata
{
    GeoMetadata() = default;
    GeoMetadata(int32_t _rows, int32_t _cols);
    GeoMetadata(int32_t _rows, int32_t _cols, double _xll, double _yll, double _cellsize, std::optional<double> _nodatavalue);

    bool operator==(const GeoMetadata& other) const noexcept;
    bool operator!=(const GeoMetadata& other) const noexcept;
    double convertXtoColFraction(const double x) const;
    double convertYtoRowFraction(const double y) const;
    int32_t convertXtoCol(const double x) const;
    int32_t convertYtoRow(const double y) const;
    double convertColCentreToX(const int32_t col) const;
    double convertRowCentreToY(const int32_t row) const;
    double convertColLLToX(const int32_t col) const;
    double convertRowLLToY(const int32_t row) const;
    Cell<int32_t> convertXYtoCell(const double x, const double y) const;
    bool isXY(const double x, const double y, const int32_t row, const int32_t col) const;
    bool isOnMap(const Cell<int32_t>& cell) const;
    bool isOnMap(const int32_t r, const int32_t c) const;
    void computeRectOnMapAround(const int32_t row, const int32_t col, const int32_t radius, int32_t& r0, int32_t& c0, int32_t& r1, int32_t& c1) const;

    std::string toString() const;

    int32_t rows    = 0;
    int32_t cols    = 0;
    double xll      = 0.0;
    double yll      = 0.0;
    double cellSize = 0.0;
    std::optional<double> nodata;
    std::string projection;
};
}
