#pragma once

#include "gdx/cell.h"
#include "gdx/rasteriterator.h"
#include "infra/cast.h"

#include <cassert>
#include <cmath>
#include <gsl/span>
#include <iterator>
#include <limits>
#include <optional>

namespace gdx {

enum class CenterCellHandling
{
    Include,
    Exclude,
};

template <CenterCellHandling center_handling>
class CircularLocationFilterPolicy
{
public:
    CircularLocationFilterPolicy() = default;
    CircularLocationFilterPolicy(Cell center, int32_t radius)
    : _center(center)
    , _radius(radius)
    {
    }

    constexpr bool exclude(Cell cell) const noexcept
    {
        if constexpr (center_handling == CenterCellHandling::Exclude) {
            if (cell == _center) {
                return true;
            }
        }

        return distance(cell, _center) > _radius;
    }

private:
    Cell _center;
    int32_t _radius = 0;
};

template <
    typename T,
    bool is_const                 = false,
    typename ValueFilterPolicy    = AllValuesFilterPolicy<T>,
    typename LocationFilterPolicy = AllLocationsFilterPolicy>
class RasterArea
{
public:
    using value_type = std::conditional_t<is_const, const T, T>;
    using pointer    = value_type*;
    using reference  = value_type&;

    RasterArea(pointer rasterTopLeft, pointer areaTopLeft, int32_t rows, int32_t cols, int32_t stride, ValueFilterPolicy valueFilter, LocationFilterPolicy locationFilter)
    : _rasterTopLeft(rasterTopLeft)
    , _areaTopLeft(areaTopLeft)
    , _rows(rows)
    , _cols(cols)
    , _stride(stride)
    , _valueFilter(valueFilter)
    , _locationFilter(locationFilter)
    {
    }

    auto begin()
    {
        return RasterIterator<value_type, is_const, ValueFilterPolicy, LocationFilterPolicy>(_rasterTopLeft, _rows, _cols, _stride, _areaTopLeft, _valueFilter, _locationFilter);
    }

    auto end()
    {
        return RasterIterator<value_type, is_const, ValueFilterPolicy, LocationFilterPolicy>();
    }

    int32_t rows() const noexcept
    {
        return _rows;
    }

    int32_t cols() const noexcept
    {
        return _rows;
    }

private:
    pointer _rasterTopLeft = nullptr;
    pointer _areaTopLeft   = nullptr;
    int32_t _rows          = 0;
    int32_t _cols          = 0;
    int32_t _stride        = 0;
    ValueFilterPolicy _valueFilter;
    LocationFilterPolicy _locationFilter;
};

namespace detail {
template <typename PointerType>
struct AreaInfo
{
    Cell topLeft;
    int32_t rows           = 0;
    int32_t cols           = 0;
    PointerType topLeftPtr = nullptr;
};

/*! Determine the area within the raster taking into
 * account the border cases
 */
template <typename Raster>
auto clip_area_to_raster(Raster&& raster, Cell center, int radius)
{
    using pointer = std::conditional_t<std::is_const_v<std::remove_reference_t<Raster>>, typename std::decay_t<Raster>::const_pointer, typename std::decay_t<Raster>::pointer>;

    AreaInfo<pointer> area;

    area.cols = radius * 2 + 1;
    area.rows = area.cols;

    // Make sure the area is within the raster region
    if (center.r - radius < 0) {
        area.rows += center.r - radius;
    }

    if (center.r + radius >= raster.rows()) {
        area.rows -= center.r + radius - raster.rows() + 1;
    }

    if (center.c - radius < 0) {
        area.cols += center.c - radius;
    }

    if (center.c + radius >= raster.cols()) {
        area.cols -= center.c + radius - raster.cols() + 1;
    }

    area.topLeft    = Cell(std::max(0, center.r - radius), std::max(0, center.c - radius));
    area.topLeftPtr = raster.data() + (area.topLeft.r * raster.cols()) + area.topLeft.c;

    assert(area.cols <= raster.cols());
    assert(area.rows <= raster.rows());

    return area;
}

template <typename Raster>
auto clip_area_to_raster(Raster&& raster, Cell topLeft, int32_t rows, int32_t cols)
{
    using pointer = std::conditional_t<std::is_const_v<std::remove_reference_t<Raster>>, typename std::decay_t<Raster>::const_pointer, typename std::decay_t<Raster>::pointer>;

    AreaInfo<pointer> area;

    area.topLeft    = topLeft;
    area.topLeftPtr = &raster[topLeft];
    area.cols       = cols;
    area.rows       = rows;

    if (area.topLeft.c + area.cols > raster.cols()) {
        area.cols = raster.cols() - area.topLeft.c;
    }

    if (area.topLeft.r + area.rows > raster.rows()) {
        area.rows = raster.rows() - area.topLeft.r;
    }

    return area;
}

}

template <
    typename Container,
    typename std::enable_if_t<std::decay_t<Container>::with_nodata, int>* = nullptr>
auto neighbouring_cells_square(Container&& raster, Cell cell, int32_t radius)
{
    using T                = typename std::decay_t<Container>::value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;

    auto areaInfo = detail::clip_area_to_raster(raster, cell, radius);
    NodataValueFilterPolicy<T> valueFilter(raster.nodata());
    SingleCellLocationFilterPolicy locationFilter(cell);
    return RasterArea<T, isConst, NodataValueFilterPolicy<T>, SingleCellLocationFilterPolicy>(raster.data(), areaInfo.topLeftPtr, areaInfo.rows, areaInfo.cols, raster.cols(), valueFilter, locationFilter);
}

template <
    typename Container,
    typename std::enable_if_t<std::decay_t<Container>::with_nodata, int>* = nullptr>
auto cells_square(Container&& raster, Cell cell, int32_t radius)
{
    using T                = typename std::decay_t<Container>::value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;

    auto areaInfo = detail::clip_area_to_raster(raster, cell, radius);
    NodataValueFilterPolicy<T> valueFilter(raster.nodata());
    return RasterArea<T, isConst, NodataValueFilterPolicy<T>>(raster.data(), areaInfo.topLeftPtr, areaInfo.rows, areaInfo.cols, raster.cols(), valueFilter, AllLocationsFilterPolicy());
}

template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto neighbouring_cells_square(Container&& raster, Cell center, int32_t radius)
{
    using T                = typename std::decay_t<Container>::value_type;
    using TMask            = typename std::decay_t<Container>::mask_value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;

    auto areaInfo = detail::clip_area_to_raster(raster, center, radius);
    NodataMaskFilterPolicy<T, TMask> valueFilter(raster.data(), raster.mask());
    SingleCellLocationFilterPolicy locationFilter(center);
    return RasterArea<T, isConst, NodataMaskFilterPolicy<T, TMask>, SingleCellLocationFilterPolicy>(raster.data(), areaInfo.topLeftPtr, areaInfo.rows, areaInfo.cols, raster.cols(), valueFilter, locationFilter);
}

template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto cells_square(Container&& raster, Cell center, int32_t radius)
{
    using T                = typename std::decay_t<Container>::value_type;
    using TMask            = typename std::decay_t<Container>::mask_value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;

    auto areaInfo = detail::clip_area_to_raster(raster, center, radius);
    NodataMaskFilterPolicy<T, TMask> valueFilter(raster.data(), raster.mask());
    return RasterArea<T, isConst, NodataMaskFilterPolicy<T, TMask>>(raster.data(), areaInfo.topLeftPtr, areaInfo.rows, areaInfo.cols, raster.cols(), valueFilter, AllLocationsFilterPolicy());
}

template <
    typename Container,
    typename pointer = std::conditional_t<std::is_const_v<Container>, typename std::decay_t<Container>::const_pointer, typename std::decay_t<Container>::pointer>>
auto neighbouring_cells_square(Container&& raster, pointer center, int32_t radius)
{
    return neighbouring_cells_square(raster, detail::raster_pointer_to_cell(raster.data(), center, raster.cols()), radius);
}

template <
    typename Container,
    typename pointer = std::conditional_t<std::is_const_v<Container>, typename std::decay_t<Container>::const_pointer, typename std::decay_t<Container>::pointer>>
auto cells_square(Container&& raster, pointer center, int32_t radius)
{
    return cells_square(raster, detail::raster_pointer_to_cell(raster.data(), center, raster.cols()), radius);
}

template <
    typename Container,
    typename std::enable_if_t<std::decay_t<Container>::with_nodata, int>* = nullptr>
auto neighbouring_cells_circular(Container&& raster, Cell center, int radius)
{
    using T                = typename std::decay_t<Container>::value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;

    auto areaInfo = detail::clip_area_to_raster(raster, center, radius);
    NodataValueFilterPolicy<T> valueFilter(raster.nodata());
    CircularLocationFilterPolicy<CenterCellHandling::Exclude> locationFilter(center, radius);

    return RasterArea<T, isConst, NodataValueFilterPolicy<T>, CircularLocationFilterPolicy<CenterCellHandling::Exclude>>(
        raster.data(),
        areaInfo.topLeftPtr,
        areaInfo.rows,
        areaInfo.cols,
        raster.cols(),
        valueFilter,
        locationFilter);
}

template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto neighbouring_cells_circular(Container&& raster, Cell center, int radius)
{
    using T                = typename std::decay_t<Container>::value_type;
    using TMask            = typename std::decay_t<Container>::mask_value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;

    auto areaInfo = detail::clip_area_to_raster(raster, center, radius);
    NodataMaskFilterPolicy<T, TMask> valueFilter(raster.data(), raster.mask());
    CircularLocationFilterPolicy<CenterCellHandling::Exclude> locationFilter(center, radius);

    return RasterArea<T, isConst, NodataMaskFilterPolicy<T, TMask>, CircularLocationFilterPolicy<CenterCellHandling::Exclude>>(
        raster.data(),
        areaInfo.topLeftPtr,
        areaInfo.rows,
        areaInfo.cols,
        raster.cols(),
        valueFilter,
        locationFilter);
}

template <typename Container>
auto neighbouring_cells_circular(Container&& raster, typename Container::pointer center, int32_t radius)
{
    return neighbouring_cells_circular(raster, detail::raster_pointer_to_cell(raster.data(), center, raster.cols()), radius);
}

template <
    typename Container,
    typename std::enable_if_t<std::decay_t<Container>::with_nodata, int>* = nullptr>
auto sub_area(Container&& raster, Cell topLeft, int32_t rows, int32_t cols)
{
    using T                = typename std::decay_t<Container>::value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;

    auto areaInfo = detail::clip_area_to_raster(raster, topLeft, rows, cols);
    return RasterArea<T, isConst, NodataValueFilterPolicy<T>>(
        raster.data(),
        areaInfo.topLeftPtr,
        areaInfo.rows,
        areaInfo.cols,
        raster.cols(),
        NodataValueFilterPolicy(raster.nodata()),
        AllLocationsFilterPolicy());
}

template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto sub_area(Container&& raster, Cell topLeft, int32_t rows, int32_t cols)
{
    using T                = typename std::decay_t<Container>::value_type;
    using TMask            = typename std::decay_t<Container>::mask_value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;

    auto areaInfo = detail::clip_area_to_raster(raster, topLeft, rows, cols);
    return RasterArea<T, isConst, NodataMaskFilterPolicy<T, TMask>>(
        raster.data(),
        areaInfo.topLeftPtr,
        areaInfo.rows,
        areaInfo.cols,
        raster.cols(),
        NodataMaskFilterPolicy(raster.data(), raster.mask()),
        AllLocationsFilterPolicy());
}
}
