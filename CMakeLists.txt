cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FindDependency)

set(PACKAGE "opaq")
set(PACKAGE_VERSION_MAJOR 0)
set(PACKAGE_VERSION_MINOR 5)
set(PACKAGE_VERSION_REVISION 0)

project(${PACKAGE})
include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 14)

option(SANITIZE_ADDRESS "Build with address sanitizer (detect invalid memory access and memory leaks)" OFF)
option(BUILD_UI "Build the user interface" OFF)
option(STATIC_BUILD "Build fully statically linked binaries" OFF)
option(STATIC_PLUGINS "Do not use shared library for plugins" OFF)

if (SANITIZE_ADDRESS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif ()

if (STATIC_BUILD)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif ()

if (MSVC)
    # use all cores for compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    # avoid windows specific warnings
    add_definitions("-D_CRT_SECURE_NO_WARNINGS")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors")
endif ()

include_directories(
    ${CMAKE_BINARY_DIR}
)

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h)

set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_subdirectory(src)
