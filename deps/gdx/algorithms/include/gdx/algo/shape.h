#pragma once

#include "gdx/line.h"
#include "gdx/point.h"
#include "infra/filesystem.h"

#include <cassert>
#include <cmath>
#include <string>
#include <vector>

namespace gdx {

namespace internal {
template <template <typename> typename RasterType, typename T>
void draw_point(RasterType<T>& ras, int32_t r, int32_t c)
{
    if (ras.metadata().is_on_map(r, c)) {
        ras(r, c) = T(1);
    }
}

template <template <typename> typename RasterType, typename T>
void steep_draw_point(RasterType<T>& ras, bool steep, int32_t i, int32_t j)
{
    if (steep) {
        draw_point(ras, i, j);
    } else {
        draw_point(ras, j, i);
    }
}

template <template <typename> typename RasterType, typename T>
void draw_line(RasterType<T>& ras, double x1, double y1, double x2, double y2)
{
    const auto& meta = ras.metadata();

    // Bresenham's line algorithm
    y1 = meta.convert_y_to_row_fraction(y1);
    x1 = meta.convert_x_to_col_fraction(x1);
    y2 = meta.convert_y_to_row_fraction(y2);
    x2 = meta.convert_x_to_col_fraction(x2);

    if (std::floor(x1) == std::floor(x2) && std::floor(y1) == std::floor(y2)) {
        internal::draw_point(ras, int(std::floor(y1)), int(std::floor(x1))); // int(x) != int(floor(x)) for negative x
        return;
    }
    const bool steep = (std::abs(std::floor(y2) - std::floor(y1)) > std::abs(std::floor(x2) - std::floor(x1)));
    // steepness in cells is != steepness in xy.  We need cell steepness
    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    const double dx = x2 - x1;
    const double dy = y2 - y1;
    const double m  = dy / dx;
    int j           = int(std::floor(y1));
    const int j2    = int(std::floor(y2));
    int i           = int(std::floor(x1));
    const int i2    = int(std::floor(x2));
    assert(i <= i2);
    // e is vertical offset, when [-1,0] its in the same row
    double e = -(1 - (y1 - j)) - (x1 - i - 0.5) * dy / dx; // e at x-halfway the cell
    assert(i2 - i >= j2 - j);
    internal::steep_draw_point(ras, steep, i, j); // always draw the beginpoint
    for (; i <= i2; ++i) {
        if (i > int(std::floor(x1)) || (x1 - i <= 0.5)) {
            if (e > 0.0) {
                j += 1;
                e -= 1.0;
            } else if (e < -1.0) {
                j -= 1;
                e += 1.0;
            }
        }
        internal::steep_draw_point(ras, steep, i, j);
        if (i + 1 < i2 || x2 - i2 >= 0.5) {
            e += m;
        } else {
            e += (0.5 + x2 - i2) * m;
        }
    }
    internal::steep_draw_point(ras, steep, i2, j2); // always draw the endpoint
}
}

void read_shape_file(const fs::path& fileName,
    std::vector<Point<double>>& point,
    std::vector<Line<double>>& lines);

template <template <typename> typename RasterType, typename T>
void draw_lines_on_raster(RasterType<T>& ras, const std::vector<Line<double>>& lines)
{
    for (auto& line : lines) {
        internal::draw_line(ras, line.start.x, line.start.y, line.end.x, line.end.y);
    }
}

template <template <typename> typename RasterType, typename T>
void draw_points_on_raster(RasterType<T>& ras, const std::vector<Point<double>>& points)
{
    auto& meta = ras.metadata();

    for (auto& point : points) {
        const int r = meta.convert_y_to_row(point.y);
        const int c = meta.convert_x_to_col(point.x);
        internal::draw_point(ras, r, c);
    }
}

template <template <typename> typename RasterType, typename T>
void draw_shapefile_on_raster(RasterType<T>& ras, const fs::path& fileName)
{
    std::vector<Point<double>> points;
    std::vector<Line<double>> lines;
    read_shape_file(fileName, points, lines);
    draw_points_on_raster(ras, points);
    draw_lines_on_raster(ras, lines);
}
}
