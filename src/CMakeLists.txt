find_path(TINYXML_INCLUDE_DIR NAMES tinyxml.h)

find_dependency(NAME TINYXML LIBNAMES tinyxml DEBUGLIBNAMES tinyxmld)

if (UNIX)
    set(hdf5_DIR ${CMAKE_PREFIX_PATH}/share/cmake)
endif ()

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Threads)

set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS system filesystem iostreams program_options)

# We use our own version of FMT, let spdlog use the same version
add_definitions(-DSPDLOG_FMT_EXTERNAL)

if (MSVC)
    set(EXTRA_LIBS Psapi Ws2_32)
endif ()

list(APPEND EXTRA_LIBS ${CMAKE_THREAD_LIBS_INIT})

find_package(Eigen3 REQUIRED)

include_directories(SYSTEM
    ${ZLIB_INCLUDE_DIR}
    ${TINYXML_INCLUDE_DIR}
)

add_definitions(-DTIXML_USE_STL -D_XOPEN_SOURCE)

if (MSVC)
    # suppress decorated name length exceeded warning
    add_definitions(/wd4503)
endif ()

add_subdirectory(plugins)
add_subdirectory(opaq_core)

if (BUILD_UI)
    add_subdirectory(ui)
endif ()

add_executable(opaq opaq.cpp)
target_link_libraries(opaq opaqcore Eigen3::Eigen Boost::program_options)

if (STATIC_PLUGINS)
    target_link_libraries(opaq ${PLUGINS})
endif ()

install(TARGETS opaq
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

