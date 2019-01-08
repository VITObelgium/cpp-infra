#pragma once

#include "gdx/cell.h"

#include <cmath>
#include <gsl/span>
#include <iterator>
#include <limits>
#include <optional>

namespace gdx {

template <typename Iter1, typename Iter2>
class NodataSkippingZipIterator
{
public:
    using difference_type   = ptrdiff_t;
    using value_type        = std::pair<typename Iter1::value_type, typename Iter2::value_type>;
    using pointer           = value_type*;
    using reference         = value_type&;
    using iterator_category = std::forward_iterator_tag;

    NodataSkippingZipIterator() = default;

    NodataSkippingZipIterator(Iter1 iter1, std::optional<typename Iter1::value_type> nodata1, Iter2 iter2, std::optional<typename Iter2::value_type> nodata2)
    : _current(std::make_pair(iter1, iter2))
    , _nodata1(nodata1)
    , _nodata2(nodata2)
    {
        skip_nodata();
    }

    NodataSkippingZipIterator(const NodataSkippingZipIterator& iter) = default;
    NodataSkippingZipIterator& operator=(const NodataSkippingZipIterator& iter) = default;

    bool operator==(const NodataSkippingZipIterator& iter) const
    {
        return _current.first == iter._current.second;
    }

    bool operator!=(const NodataSkippingZipIterator& iter) const
    {
        return !(*this == iter);
    }

    reference operator*() const
    {
        return *_current.first;
    }

    pointer operator->() const
    {
        return _current.first;
    }

    NodataSkippingZipIterator& operator++()
    {
        increment();
        skip_nodata();
        return *this;
    }

    NodataSkippingZipIterator operator++(int)
    {
        NodataSkippingZipIterator<T> iter = *this;
        increment();
        skip_nodata();
        return iter;
    }

private:
    void increment()
    {
        ++_current.first;
        ++_current.second.c;

        if (_current.second.c >= _cols) {
            _current.second.c = 0;
            ++_current.second.r;
        }
    }

    void skip_nodata()
    {
        while (_current.first < _end && is_nodata(*_current.first)) {
            increment();
        }

        if (_current.first >= _end) {
            _current.first = nullptr;
        }
    }

    bool is_nodata1(typename Iter1::value_type val)
    {
        if constexpr (std::numeric_limits<typename Iter1::value_type>::has_quiet_NaN) {
            return std::isnan(val);
        } else {
            return _nodata1 == val;
        }
    }

    bool is_nodata2(typename Iter1::value_type val)
    {
        if constexpr (std::numeric_limits<typename Iter2::value_type>::has_quiet_NaN) {
            return std::isnan(val);
        } else {
            return _nodata2 == val;
        }
    }

    value_type _current;
    std::optional<typename Iter1::value_type> _nodata1;
    std::optional<typename Iter2::value_type> _nodata2;
};

// NoData concept for a container type C
// C::nodata_type                               type of the nodata value
// C::nodata_type C::nodata() const noexcept    function that returns the nodata value

// supports skipping over containers that implement the nodata concept
template <
    typename Container1,
    typename Container2,
    typename NodataType1 = typename std::decay_t<Container1>::nodata_type,
    typename NodataType2 = typename std::decay_t<Container2>::nodata_type>
auto zip_begin(Container1&& data1, Container2&& data2)
{
    return NodataSkippingZipIterator(begin(data1), data1.nodata(), begin(data2), data2.nodata());
}

template <typename Container1, typename Container2>
auto zip_end(Container1&& data1, Container1&& data2)
{
    return NodataSkippingZipIterator(data1, data2);
}
}
