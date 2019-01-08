#pragma once

#include "infra/cast.h"
#include "util/cell.h"

#include <cassert>
#include <cmath>
#include <gsl/span>
#include <iterator>
#include <limits>
#include <optional>

namespace gdx {

template <typename T, bool is_const = false>
class NodataSkippingIterator
{
public:
    using difference_type   = ptrdiff_t;
    using value_type        = std::remove_cv_t<T>;
    using pointer           = std::conditional_t<is_const, const value_type*, value_type*>;
    using reference         = std::conditional_t<is_const, const value_type&, value_type&>;
    using iterator_category = std::forward_iterator_tag;

    NodataSkippingIterator() = default;

    NodataSkippingIterator(pointer data, size_t size, std::optional<double> nodata) noexcept
    : _current(data)
    , _end(data + size)
    , _nodata(inf::optional_cast<value_type>(nodata))
    {
        skipNodata();
    }

    NodataSkippingIterator(const NodataSkippingIterator& iter) = default;
    NodataSkippingIterator& operator=(const NodataSkippingIterator& iter) = default;

    bool operator==(const NodataSkippingIterator& iter) const noexcept
    {
        return _current == iter._current;
    }

    bool operator!=(const NodataSkippingIterator& iter) const noexcept
    {
        return !(*this == iter);
    }

    reference operator*() const noexcept
    {
        return *_current;
    }

    NodataSkippingIterator& operator++() noexcept
    {
        increment();
        skipNodata();
        return *this;
    }

    NodataSkippingIterator operator++(int) noexcept
    {
        NodataSkippingIterator<T> iter = *this;
        increment();
        skipNodata();
        return iter;
    }

private:
    void increment() noexcept
    {
        ++_current;
    }

    void skipNodata() noexcept
    {
        while (_current < _end && is_nodata()) {
            increment();
        }

        if (_current >= _end) {
            _current = nullptr;
        }
    }

    bool is_nodata() noexcept
    {
        if (!_current) {
            return false;
        }

        if constexpr (std::numeric_limits<value_type>::has_quiet_NaN) {
            return std::isnan(*_current);
        } else {
            return *_current == _nodata;
        }
    }

    pointer _current                  = nullptr;
    pointer _end                      = nullptr;
    std::optional<value_type> _nodata = nullptr;
};

template <typename T, bool is_const = false>
struct MaskValueProxy
{
    using pointer      = std::conditional_t<is_const, const T*, T*>;
    using mask_pointer = std::conditional_t<is_const, const bool*, bool*>;

    MaskValueProxy() = default;
    MaskValueProxy(pointer val, mask_pointer maskPtr) noexcept
    : value(val)
    , mask(maskPtr)
    {
    }

    operator T() const noexcept
    {
        return *value;
    }

    operator bool() const noexcept
    {
        return !is_nodata();
    }

    MaskValueProxy<T, is_const>& operator=(T val) noexcept
    {
        *value = val;
        if (mask) {
            *mask = false;
        }
        return *this;
    }

    MaskValueProxy<T, is_const>& operator=(const std::optional<T>& val) noexcept
    {
        if (val.has_value()) {
            *value = val.value();
            if (mask) {
                *mask = false;
            }
        } else {
            assert(mask);
            *mask = true;
        }
        return *this;
    }

    bool is_nodata() const noexcept
    {
        return mask ? *mask : false;
    }

    void mark_as_nodata() noexcept
    {
        assert(mask);
        *mask = true;
    }

    pointer value     = nullptr;
    mask_pointer mask = nullptr;
};

template <typename T, bool is_const = false>
class MissingValueMaskIterator
{
public:
    using difference_type   = ptrdiff_t;
    using value_type        = std::remove_cv_t<T>;
    using value_type_ptr    = std::conditional_t<is_const, const value_type*, value_type*>;
    using reference         = std::conditional_t<is_const, const MaskValueProxy<T, is_const>&, MaskValueProxy<T, is_const>&>;
    using pointer           = std::conditional_t<is_const, const MaskValueProxy<T, is_const>*, MaskValueProxy<T, is_const>*>;
    using mask_pointer      = std::conditional_t<is_const, const bool*, bool*>;
    using iterator_category = std::forward_iterator_tag;

    MissingValueMaskIterator() = default;

    MissingValueMaskIterator(value_type_ptr data, mask_pointer mask) noexcept
    : _current(data, mask)
    {
    }

    MissingValueMaskIterator(const MissingValueMaskIterator& iter) = default;
    MissingValueMaskIterator& operator=(const MissingValueMaskIterator& iter) = default;

    bool operator==(const MissingValueMaskIterator& iter) const noexcept
    {
        return _current.value == iter._current.value;
    }

    bool operator!=(const MissingValueMaskIterator& iter) const noexcept
    {
        return !(*this == iter);
    }

    reference operator*() noexcept
    {
        return _current;
    }

    pointer operator->() noexcept
    {
        return &_current;
    }

    MissingValueMaskIterator& operator++() noexcept
    {
        increment();
        return *this;
    }

    MissingValueMaskIterator operator++(int) noexcept
    {
        MissingValueMaskIterator<T> iter = *this;
        increment();
        return iter;
    }

private:
    void increment() noexcept
    {
        ++_current.value;
        if (_current.mask) {
            ++_current.mask;
        }
    }

    MaskValueProxy<value_type, is_const> _current;
};

template <typename T>
using NodataSkippingConstIterator = NodataSkippingIterator<T, true>;
template <typename T>
using MissingValueMaskConstIterator = MissingValueMaskIterator<T, true>;

// NoData concept for a container type C
// C::mask_value_type                                   type of the nodata mask
// const mask_value_type* C::mask() const noexcept      function that returns the mask pointer

// supports skipping over containers that implement the masked nodata concept
template <typename Container>
auto value_begin(Container&& rasterData)
{
    using T                = typename std::decay_t<Container>::value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;
    return NodataSkippingIterator<T, isConst>(data(rasterData), size(rasterData), rasterData.mask());
}

template <typename Container>
auto value_end(Container&& rasterData)
{
    using T                = typename std::decay_t<Container>::value_type;
    constexpr bool isConst = std::is_const_v<std::remove_reference_t<Container>>;
    return NodataSkippingIterator<T, isConst>(data(rasterData) + size(rasterData) + 1, 0, nullptr);
}

template <typename Container>
auto value_cbegin(Container&& data)
{
    using T = typename std::decay_t<Container>::value_type;
    return NodataSkippingConstIterator<T>(data.data(), data.size(), data.mask());
}

template <typename Container>
auto value_cend(const Container& data)
{
    using T = typename std::decay_t<Container>::value_type;
    return NodataSkippingConstIterator<T>(data.data() + data.size(), 0, nullptr);
}

// supports skipping over containers that implement the masked nodata concept
template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto optional_value_begin(const Container& rasterData)
{
    using T = typename std::decay_t<Container>::value_type;
    return MissingValueMaskIterator<T, true>(data(rasterData), rasterData.mask());
}

template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto optional_value_begin(Container& rasterData)
{
    using T = typename std::decay_t<Container>::value_type;
    return MissingValueMaskIterator<T, false>(data(rasterData), rasterData.mask());
}

template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto optional_value_end(const Container& rasterData)
{
    using T = typename std::decay_t<Container>::value_type;
    return MissingValueMaskIterator<T, true>(data(rasterData) + size(rasterData), nullptr);
}

template <
    typename Container,
    typename std::enable_if_t<std::is_arithmetic_v<typename std::decay_t<Container>::mask_value_type>, int>* = nullptr>
auto optional_value_end(Container& rasterData)
{
    using T = typename std::decay_t<Container>::value_type;
    return MissingValueMaskIterator<T, false>(data(rasterData) + size(rasterData), nullptr);
}
}
