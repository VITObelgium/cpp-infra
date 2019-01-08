#pragma once

#include <Eigen/Core>
#include <iterator>
#include <type_traits>

namespace Eigen {
template <typename T>
auto cbegin(const Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& arr)
{
    return arr.data();
}

template <typename T>
auto cend(const Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& arr)
{
    return arr.data() + arr.size();
}

template <typename T>
auto begin(Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& arr)
{
    return arr.data();
}

template <typename T>
auto begin(const Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& arr)
{
    return arr.data();
}

template <typename T>
auto end(Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& arr)
{
    return arr.data() + arr.size();
}

template <typename T>
auto end(const Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& arr)
{
    return arr.data() + arr.size();
}

template <typename T>
auto* data(const Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& arr)
{
    return arr.data();
}

template <typename T>
auto* data(Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& arr)
{
    return arr.data();
}

template <typename T>
auto size(const Eigen::Array<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>& arr)
{
    return arr.size();
}

}
