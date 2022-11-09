#pragma once

// We prefer to use the tbb runtime for the parallel stl implementation
// so we can register thread start and stop callback
// gcc uses intels pstl implementation under the hood
// which depends on tbb, so that's fine
// MSVC does not rely on the tbb so we use pstl manually in that case
// and do not rely on the std implentation

#include <ciso646> // this allows us to check for libcpp

#if __has_include(<execution>)
    #if defined(_LIBCPP_VERSION) && defined(_LIBCPP_HAS_PARALLEL_ALGORITHMS)
        #define INF_HAS_STD_EXECUTION 1
    #endif

    #if defined(__GLIBCXX__)
        #define INF_HAS_STD_EXECUTION 1
    #endif
#endif

#ifndef INF_HAS_STD_EXECUTION
    #define INF_HAS_STD_EXECUTION 0
#endif

#if INF_HAS_STD_EXECUTION
    #include <algorithm>
    #include <execution>
#else
    #ifndef Q_MOC_RUN // QTBUG-80990
        #include <oneapi/dpl/execution>
        #include <oneapi/dpl/algorithm>

        #ifdef _MSC_VER
            namespace std::execution {
            using namespace oneapi::dpl::execution;
            }
        #endif
    #endif
#endif
