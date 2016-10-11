find_path(ZLIB_INCLUDE_DIR NAMES zlib.h)
find_library(ZLIB_LIBRARY NAMES zlibstatic z)
find_library(ZLIB_LIBRARY_DEBUG NAMES zlibstaticd z)

find_path(TINYXML_INCLUDE_DIR NAMES tinyxml.h)
find_library(TINYXML_LIBRARY NAMES tinyxml)
find_library(TINYXML_LIBRARY_DEBUG NAMES tinyxml_d)

find_library(GZSTREAM_LIBRARY NAMES gzstream)
find_library(GZSTREAM_LIBRARY_DEBUG NAMES gzstream_d)

if (UNIX)
    set(hdf5_DIR ${CMAKE_PREFIX_PATH}/share/cmake)
endif ()

#find_package(HDF5 NAMES hdf5 COMPONENTS C CPP static)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(hdf5 CONFIG REQUIRED)

find_package(Boost REQUIRED COMPONENTS system filesystem)

# We use our own version of FMT, let spdlog use the same version
add_definitions(-DSPDLOG_FMT_EXTERNAL)

if (MSVC)
    find_library(GETOPT_LIBRARY NAMES wingetopt)
    find_library(GETOPT_LIBRARY_DEBUG NAMES wingetopt_d)
    find_library(DL_LIBRARY NAMES dl)
    find_library(DL_LIBRARY_DEBUG NAMES dl_d)

    set(EXTRA_LIBS Psapi Ws2_32)
endif ()

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
