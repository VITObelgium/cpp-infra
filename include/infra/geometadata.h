#pragma once

#include "infra/cell.h"
#include "infra/coordinate.h"
#include "infra/point.h"
#include "infra/rect.h"
#include "infra/size.h"

#include <array>
#include <fmt/core.h>
#include <optional>
#include <sstream>

namespace inf {

struct GeoMetadata
{
    struct CellSize
    {
        constexpr CellSize() noexcept = default;
        constexpr CellSize(double x_, double y_) noexcept
        : x(x_), y(y_)
        {
        }

        bool is_valid() const noexcept
        {
            return !std::isnan(x) && !std::isnan(y);
        }

        bool operator==(const CellSize& rhs) const noexcept
        {
            if (!is_valid() && !rhs.is_valid()) {
                return true;
            }

            return x == rhs.x && y == rhs.y;
        }

        bool operator!=(const CellSize& rhs) const noexcept
        {
            return !(*this == rhs);
        }

        CellSize& operator*=(double factor) noexcept
        {
            x *= factor;
            y *= factor;
            return *this;
        }

        CellSize& operator/=(double factor) noexcept
        {
            x /= factor;
            y /= factor;
            return *this;
        }

        double x = std::numeric_limits<double>::quiet_NaN();
        double y = std::numeric_limits<double>::quiet_NaN();
    };

    GeoMetadata() = default;
    GeoMetadata(int32_t rows, int32_t cols);
    GeoMetadata(int32_t rows, int32_t cols, std::optional<double> nodatavalue);
    GeoMetadata(int32_t rows, int32_t cols, double xll, double yll, double cellsize, std::optional<double> nodatavalue);
    GeoMetadata(int32_t rows, int32_t cols, double xll, double yll, double cellsize, std::optional<double> nodatavalue, std::string_view projection);

    // Different cellsize in x and y direction
    GeoMetadata(int32_t rows, int32_t cols, double xll, double yll, CellSize cellsize, std::optional<double> nodatavalue);
    GeoMetadata(int32_t rows, int32_t cols, double xll, double yll, CellSize cellsize, std::optional<double> nodatavalue, std::string_view projection);

    bool is_north_up() const noexcept;

    void set_cell_size(double size) noexcept;
    double cell_size_x() const noexcept;
    double cell_size_y() const noexcept;

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
    Point<double> convert_cell_ll_to_xy(const Cell& cell) const;
    Cell convert_xy_to_cell(double x, double y) const;
    Cell convert_coordinate_to_cell(const Coordinate& coord) const;

    bool is_xy(double x, double y, int32_t row, int32_t col) const;

    template <typename T>
    bool is_on_map(const Point<T>& point) const
    {
        return is_on_map(convert_xy_to_cell(static_cast<double>(point.x), static_cast<double>(point.y)));
    }

    bool is_on_map(const Cell& cell) const;
    bool is_on_map(int32_t r, int32_t c) const;
    void compute_rect_on_map_around(int32_t row, int32_t col, int32_t radius, int32_t& r0, int32_t& c0, int32_t& r1, int32_t& c1) const;

    Rect<double> bounding_box() const noexcept;
    Rect<double> bounding_box(const Cell& cell) const noexcept;

    template <typename T>
    Cell convert_point_to_cell(const Point<T> point) const
    {
        return Cell(convert_y_to_row(static_cast<double>(point.y)), convert_x_to_col(static_cast<double>(point.x)));
    }

    Point<double> center() const;
    Point<double> top_left() const;
    Point<double> bottom_right() const;
    Point<double> top_right() const;
    Point<double> bottom_left() const;

    Point<double> top_left_center() const;

    std::string to_string() const;

    double width() const noexcept;
    double height() const noexcept;

    std::optional<int32_t> geographic_epsg() const noexcept;
    std::optional<int32_t> projected_epsg() const noexcept;
    std::string projection_frienly_name() const noexcept;
    void set_projection_from_epsg(int32_t epsg);

    int32_t rows = 0;
    int32_t cols = 0;
    double xll   = 0.0;
    double yll   = 0.0;
    CellSize cellSize;
    std::optional<double> nodata;
    std::string projection;
};

std::array<double, 6> metadata_to_geo_transform(const GeoMetadata& meta);
std::ostream& operator<<(std::ostream& os, const GeoMetadata& meta);

GeoMetadata copy_metadata_replace_nodata(const GeoMetadata& meta, std::optional<double> nodata);

bool metadata_intersects(const GeoMetadata& meta1, const GeoMetadata& meta2);
bool metadata_is_aligned(const GeoMetadata& meta1, const GeoMetadata& meta2) noexcept;

GeoMetadata metadata_intersection(const GeoMetadata& meta1, const GeoMetadata& meta2);

}

template <>
struct fmt::formatter<inf::GeoMetadata::CellSize>
{
    FMT_CONSTEXPR20 auto parse(format_parse_context& ctx) -> format_parse_context::iterator
    {
        return ctx.begin();
    }

    auto format(const inf::GeoMetadata::CellSize& s, format_context& ctx) const -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "({}x{})", s.x, s.y);
    }
};
