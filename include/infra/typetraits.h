#pragma once

#include <type_traits>

namespace inf {

template <typename T>
struct dependent_false : std::false_type
{
};

template <typename T>
struct dependent_true : std::true_type
{
};

template <typename T>
inline constexpr bool dependent_false_v = dependent_false<T>::value;

template <typename T>
inline constexpr bool dependent_true_v = dependent_true<T>::value;

}
