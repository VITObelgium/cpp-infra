#pragma once

#if __has_include(<span>) && (__cplusplus > 201703L)
#include <span>
#else
#include <gsl/span>
namespace std {
template <class T, std::size_t Extent = gsl::dynamic_extent>
using span = gsl::span<T, Extent>;
}
#endif
