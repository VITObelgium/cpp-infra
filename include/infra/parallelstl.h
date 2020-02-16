#pragma once

// We prefer to use the tbb runtime for the parallel stl implementation
// so we can register thread start and stop callback
// gcc uses intels pstl implementation under the hood
// which depends on tbb, so that's fine
// MSVC does not rely on the tbb so we use pstl manually in that case
// and do not rely on the std implentation
#if __has_include(<execution>) && !defined(_MSC_VER)
#include <algorithm>
#include <execution>
#else
#ifndef Q_MOC_RUN //QTBUG-80990
#include <pstl/algorithm>
#include <pstl/execution>
#endif
#ifdef _MSC_VER
namespace std::execution {
using namespace __pstl::execution;
}
#endif
#endif
