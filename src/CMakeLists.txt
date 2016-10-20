find_path(ZLIB_INCLUDE_DIR NAMES zlib.h)
find_path(TINYXML_INCLUDE_DIR NAMES tinyxml.h)

find_dependency(NAME ZLIB LIBNAMES zlibstatic z DEBUGLIBNAMES zlibstaticd)
find_dependency(NAME TINYXML LIBNAMES tinyxml DEBUGLIBNAMES tinyxml_d)

if (UNIX)
    set(hdf5_DIR ${CMAKE_PREFIX_PATH}/share/cmake)
endif ()

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(hdf5 CONFIG)
find_package(Threads)

set (Boost_USE_STATIC_LIBS ON)
add_definitions("-DBOOST_ALL_NO_LIB")
find_package(Boost REQUIRED COMPONENTS system filesystem iostreams program_options)

# We use our own version of FMT, let spdlog use the same version
add_definitions(-DSPDLOG_FMT_EXTERNAL)

if (MSVC)
    set(EXTRA_LIBS Psapi Ws2_32)
endif ()

list(APPEND EXTRA_LIBS ${CMAKE_THREAD_LIBS_INIT})

find_path(EIGEN3_INCLUDE_DIR
    NAMES Eigen/Dense
    PATH_SUFFIXES eigen3
)

if (NOT EIGEN3_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find eigen headers")
endif ()

include_directories(SYSTEM
    ${EIGEN3_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${TINYXML_INCLUDE_DIR}
)

add_definitions(-DTIXML_USE_STL -D_XOPEN_SOURCE)

if (MSVC)
    # suppress decorated name length exceeded warning
    add_definitions(/wd4503)
endif ()

add_subdirectory(opaq_core)
add_subdirectory(plugins)

if (BUILD_UI)
    add_subdirectory(ui)
endif ()
