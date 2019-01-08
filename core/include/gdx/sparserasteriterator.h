#pragma once

#include <Eigen/SparseCore>
#include <cassert>
#include <iterator>

namespace gdx {
/*! Iterator that iterates over a sparse matrix and returns nodata values for the cells
 *  that do not have a value
 */
template <typename T, bool is_const = false>
class SparseMatrixIterator
{
public:
    using value_type        = T;
    using pointer           = std::conditional_t<is_const, const value_type*, value_type*>;
    using reference         = std::conditional_t<is_const, const value_type&, value_type&>;
    using matrix_type       = std::conditional_t<is_const, const Eigen::SparseMatrix<T, Eigen::RowMajor>, Eigen::SparseMatrix<T, Eigen::RowMajor>>;
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = size_t;

    SparseMatrixIterator() = default;

    SparseMatrixIterator(matrix_type& mat, T nodata)
    : _mat(&mat)
    , _nodata(nodata)
    , _currentValue(&_nodata)
    , _iter(mat, 0)
    , _row(0)
    , _col(0)
    {
        if (!_iter) {
            // no values on the first row
            advance_iter();
        } else {
            if (_iter.row() == 0 && _iter.col() == 0) {
                _currentValue = &_iter.valueRef();
                advance_iter();
            }
        }
    }

    SparseMatrixIterator(const SparseMatrixIterator& iter) = default;
    SparseMatrixIterator& operator=(const SparseMatrixIterator& iter) = default;

    bool operator==(const SparseMatrixIterator& iter) const
    {
        return _row == iter._row && _col == iter._col;
    }

    bool operator!=(const SparseMatrixIterator& iter) const
    {
        return !(*this == iter);
    }

    reference operator*() const
    {
        assert(_currentValue);
        return *_currentValue;
    }

    SparseMatrixIterator& operator++()
    {
        increment();
        return *this;
    }

    SparseMatrixIterator operator++(int)
    {
        SparseMatrixIterator<T> iter = *this;
        increment();
        return iter;
    }

private:
    void increment()
    {
        ++_col;
        if (_col == _mat->cols()) {
            _col = 0;
            ++_row;
        }

        if (_row == _mat->rows()) {
            _row          = -1;
            _col          = -1;
            _currentValue = nullptr;
        }

        if (_row == _iter.row() && _col == _iter.col()) {
            _currentValue = &_iter.valueRef();
            advance_iter();
        } else {
            _currentValue = &_nodata;
        }
    }

    void advance_iter()
    {
        if (_iter) {
            ++_iter;
        }

        auto row = _iter.row();

        // Advance to a row that contains data
        while (row < _mat->rows() && !_iter) {
            ++row;
            _iter = decltype(_iter)(*_mat, row);
        }
    }

    matrix_type* _mat = nullptr;
    value_type _nodata;
    pointer _currentValue = nullptr;
    typename std::decay_t<matrix_type>::InnerIterator _iter;
    int _row = -1;
    int _col = -1;
};

/*! Iterator that iterates over sparse matrix elements
 * only the actual values will be iterated
 */
template <typename T, bool is_const = false>
class SparseMatrixValueIterator
{
public:
    using value_type        = T;
    using pointer           = std::conditional_t<is_const, const value_type*, value_type*>;
    using reference         = std::conditional_t<is_const, const value_type&, value_type&>;
    using matrix_type       = std::conditional_t<is_const, const Eigen::SparseMatrix<T, Eigen::RowMajor>, Eigen::SparseMatrix<T, Eigen::RowMajor>>;
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = size_t;

    SparseMatrixValueIterator() = default;

    SparseMatrixValueIterator(matrix_type& mat)
    : _mat(&mat)
    , _iter(mat, 0)
    , _row(0)
    {
        if (!_iter) {
            // no values on the first row
            increment();
        }
    }

    SparseMatrixValueIterator(const SparseMatrixValueIterator& iter) = default;
    SparseMatrixValueIterator& operator=(const SparseMatrixValueIterator& iter) = default;

    bool operator==(const SparseMatrixValueIterator& iter) const
    {
        if (_row == -1 || iter._row == -1) {
            return _row == iter._row;
        }

        return _iter == iter._iter;
    }

    bool operator!=(const SparseMatrixValueIterator& iter) const
    {
        return !(*this == iter);
    }

    reference operator*()
    {
        assert(_iter);
        return _iter.valueRef();
    }

    SparseMatrixValueIterator& operator++()
    {
        increment();
        return *this;
    }

    SparseMatrixValueIterator operator++(int)
    {
        SparseMatrixIterator<T> iter = *this;
        increment();
        return iter;
    }

private:
    void increment()
    {
        ++_iter;
        while (_row < _mat->rows() && !_iter) {
            ++_row;
            _iter = decltype(_iter)(*_mat, _row);
        }

        if (_row == _mat->rows()) {
            _row = -1;
        }
    }

    matrix_type* _mat = nullptr;
    typename std::decay_t<matrix_type>::InnerIterator _iter;
    int _row = -1;
};
}
