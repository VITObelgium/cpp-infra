cmake_minimum_required(VERSION 3.12)

set(PACKAGE_VERSION_COMMITHASH "dev" CACHE STRING "git commit hash")
if (PACKAGE_VERSION_COMMITHASH STREQUAL "dev")
    set(GDX_DEV_BUILD 1)
else ()
    set(GDX_DEV_BUILD 0)
endif ()

enable_testing()

include(GNUInstallDirs)
include(CTestUseLaunchers)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/gdx)

option(GDX_DISABLE_OPENMP "Build without openmp acceleration support" OFF)
option(GDX_ENABLE_TOOLS "Build the gdx tools" ON)
option(GDX_ENABLE_TESTS "Build the unit tests" ON)
option(GDX_ENABLE_TEST_UTILS "Build the unit test utilities" ON)
option(GDX_ENABLE_BENCHMARKS "Build the micro benchmarks" OFF)
option(GDX_PYTHON_BINDINGS "Build python bindings" ON)
option(GDX_INSTALL_DEVELOPMENT_FILES "Install the gdx development files (headers/libs)" ON)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" GDX_IS_TOPLEVEL)

if (GDX_IS_TOPLEVEL)
    option(ENABLE_ADDRESS_SANITIZER "Enable the address sanitizer" OFF)
    option(ENABLE_UB_SANITIZER "Enable the undefined behavior sanitizer" OFF)
    
    project(GDX
        VERSION 0.5.0
        LANGUAGES CXX
    )
    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

    message(STATUS "Gdx version: ${GDX_VERSION} (${PACKAGE_VERSION_COMMITHASH})")
    message(STATUS "Unit tests: ${GDX_ENABLE_TESTS}")
    message(STATUS "Address sanitizer: ${ENABLE_ADDRESS_SANITIZER}")
    message(STATUS "Undefined behavior sanitizer: ${ENABLE_UB_SANITIZER}")

    if (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:twoPhase- /permissive- /bigobj /w34100 /w34101 /w35038 /w34127")
        # avoid windows specific warnings
        add_definitions(
            -D_CRT_SECURE_NO_WARNINGS
            -D_SCL_SECURE_NO_WARNINGS
            -D_SILENCE_CXX17_NEGATORS_DEPRECATION_WARNING
            -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
            -D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
        )

        # Fixes warning when linking against release c library in debug mode
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT")
        set(CMAKE_STATIC_LINKER_FLAGS_DEBUG "${CMAKE_STATIC_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:MSVCRT")

        # Enable faster linking for debug builds
        set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG:FASTLINK")
        set (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG:FASTLINK")
        set (CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} /DEBUG:FASTLINK")
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wfatal-errors -Wno-unknown-pragmas -fdiagnostics-color -Wno-gnu-zero-variadic-macro-arguments")
        if (CMAKE_COMPILER_IS_GNUCXX)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-maybe-uninitialized")
        endif ()
    endif ()

    if (NOT MSVC AND ENABLE_ADDRESS_SANITIZER)
        set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -fno-optimize-sibling-calls -fno-omit-frame-pointer -fsanitize=address")
        set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls")
        set (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls")
        set (CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fno-optimize-sibling-calls")

        if (${CMAKE_COMPILER_IS_GNUCXX})
            set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -static-libasan")
            set (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -static-libasan")
            set (CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -static-libasan")
        endif ()
    endif ()

    if (NOT MSVC AND ENABLE_UB_SANITIZER)
        set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
        set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=undefined")
        set (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=undefined")
        set (CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fsanitize=undefined")
    endif ()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND NOT MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif ()

    write_basic_package_version_file("GdxConfigVersion.cmake"
        VERSION ${GDX_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/GdxConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/GdxConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
    )

    if (GDX_INSTALL_DEVELOPMENT_FILES)
        install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/GdxConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/GdxConfigVersion.cmake
            DESTINATION ${INSTALL_CONFIGDIR}
        )
    endif ()
endif ()

if (MINGW OR UNIX)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

if (MINGW)
    add_definitions("-DMINGW_HAS_SECURE_API")
endif ()

if (MSVC)
    set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
    set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
else ()
    set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
    set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
endif ()

find_package(Gsl REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

if (NOT GDX_DISABLE_OPENMP)
    find_package(OpenMP)
endif ()

message(STATUS "Gdx OpenMP support: ${OpenMP_FOUND}")

if (GDX_ENABLE_TEST_UTILS OR GDX_ENABLE_TESTS)
    find_package(GTest REQUIRED)
    find_package(GMock REQUIRED)
endif ()

if (GDX_ENABLE_TEST_UTILS)
    add_subdirectory(testutil)
endif ()

if (GDX_IS_TOPLEVEL)
    # gdx is the toplevel project so include the infra dependency
    set(INFRA_EMBED_GDAL_DATA ON CACHE BOOL "" FORCE)
    set(INFRA_INSTALL_DEVELOPMENT_FILES ${GDX_INSTALL_DEVELOPMENT_FILES} CACHE BOOL "" FORCE)
    add_subdirectory(deps/infra)
endif ()

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(algorithms)

if (GDX_PYTHON_BINDINGS)
    add_subdirectory(python)
    if (GDX_ENABLE_TESTS)
        add_subdirectory(test)
    endif ()
endif ()

if (GDX_ENABLE_TOOLS)
    add_subdirectory(tools)
endif ()

if (GDX_ENABLE_BENCHMARKS)
    find_package(benchmark REQUIRED)
    add_subdirectory(benchmarks)
endif ()


if (EMSCRIPTEN)
    add_subdirectory(js)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gdxconfig.h.in ${CMAKE_BINARY_DIR}/gdxconfig.h)
if (WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc @ONLY)
endif ()

if (GDX_IS_TOPLEVEL AND GDX_INSTALL_DEVELOPMENT_FILES)
    install(EXPORT gdx-targets
        FILE GdxTargets.cmake
        NAMESPACE Gdx::
        DESTINATION ${INSTALL_CONFIGDIR}
    )
endif ()
