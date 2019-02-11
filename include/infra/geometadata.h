#pragma once

#include "infra/cell.h"
#include "infra/coordinate.h"
#include "infra/point.h"

#include <array>
#include <optional>
#include <sstream>

namespace inf {

struct GeoMetadata
{
    GeoMetadata() = default;
    GeoMetadata(int32_t _rows, int32_t _cols);
    GeoMetadata(int32_t _rows, int32_t _cols, std::optional<double> _nodatavalue);
    GeoMetadata(int32_t _rows, int32_t _cols, double _xll, double _yll, double _cellsize, std::optional<double> _nodatavalue);

    bool operator==(const GeoMetadata& other) const noexcept;
    bool operator!=(const GeoMetadata& other) const noexcept;
    double convert_x_to_col_fraction(double x) const;
    double convert_y_to_row_fraction(double y) const;
    int32_t convert_x_to_col(double x) const;
    int32_t convert_y_to_row(double y) const;
    double convert_col_centre_to_x(int32_t col) const;
    double convert_row_centre_to_y(int32_t row) const;
    Point<double> convert_cell_centre_to_xy(const Cell& cell) const;
    double convert_col_ll_to_x(int32_t col) const;
    double convert_row_ll_to_y(int32_t row) const;
    Cell convert_xy_to_cell(double x, double y) const;
    Cell convert_coordinate_to_cell(const Coordinate& coord) const;
    bool is_xy(double x, double y, int32_t row, int32_t col) const;
    bool is_on_map(const Cell& cell) const;
    bool is_on_map(int32_t r, int32_t c) const;
    void compute_rect_on_map_around(int32_t row, int32_t col, int32_t radius, int32_t& r0, int32_t& c0, int32_t& r1, int32_t& c1) const;

    template <typename T>
    Cell convert_point_to_cell(const Point<T> point) const
    {
        return Cell(convert_y_to_row(point.y), convert_x_to_col(point.x));
    }

    Point<double> center() const;
    Point<double> top_left() const;
    Point<double> bottom_right() const;

    std::string to_string() const;

    std::optional<int32_t> projection_geo_epsg() const;
    std::optional<int32_t> projection_epsg() const;
    std::string projection_frienly_name() const;
    void set_projection_from_epsg(int32_t epsg);

    int32_t rows    = 0;
    int32_t cols    = 0;
    double xll      = 0.0;
    double yll      = 0.0;
    double cellSize = 0.0;
    std::optional<double> nodata;
    std::string projection;
};

std::array<double, 6> metadata_to_geo_transform(const GeoMetadata& meta);
std::ostream& operator<<(std::ostream& os, const GeoMetadata& meta);
}
