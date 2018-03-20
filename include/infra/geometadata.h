#pragma once

#include "infra/cell.h"
#include "infra/point.h"

#include <array>
#include <optional>
#include <sstream>

namespace infra {

struct GeoMetadata
{
    GeoMetadata() = default;
    GeoMetadata(int32_t _rows, int32_t _cols);
    GeoMetadata(int32_t _rows, int32_t _cols, std::optional<double> _nodatavalue);
    GeoMetadata(int32_t _rows, int32_t _cols, double _xll, double _yll, double _cellsize, std::optional<double> _nodatavalue);

    bool operator==(const GeoMetadata& other) const noexcept;
    bool operator!=(const GeoMetadata& other) const noexcept;
    double convertXtoColFraction(double x) const;
    double convertYtoRowFraction(double y) const;
    int32_t convertXtoCol(double x) const;
    int32_t convertYtoRow(double y) const;
    double convertColCentreToX(int32_t col) const;
    double convertRowCentreToY(int32_t row) const;
    double convertColLLToX(int32_t col) const;
    double convertRowLLToY(int32_t row) const;
    Cell<int32_t> convertXYtoCell(double x, double y) const;
    bool isXY(double x, double y, int32_t row, int32_t col) const;
    bool isOnMap(const Cell<int32_t>& cell) const;
    bool isOnMap(int32_t r, int32_t c) const;
    void computeRectOnMapAround(int32_t row, int32_t col, int32_t radius, int32_t& r0, int32_t& c0, int32_t& r1, int32_t& c1) const;

    Point<double> center() const;
    Point<double> topLeft() const;
    Point<double> bottomRight() const;

    std::string toString() const;

    std::optional<int32_t> projectionGeoEpsg() const;
    std::optional<int32_t> projectionEpsg() const;
    std::string projectionFrienlyName() const;
    void setProjectionFromEpsg(int32_t epsg);

    int32_t rows    = 0;
    int32_t cols    = 0;
    double xll      = 0.0;
    double yll      = 0.0;
    double cellSize = 0.0;
    std::optional<double> nodata;
    std::string projection;
};

std::array<double, 6> metadataToGeoTransform(const GeoMetadata& meta);
std::ostream& operator<<(std::ostream& os, const GeoMetadata& meta);
}
