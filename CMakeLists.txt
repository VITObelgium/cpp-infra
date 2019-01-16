cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)
cmake_policy(SET CMP0071 NEW)

set(PACKAGE "OPAQ")
set(PACKAGE_VERSION_COMMITHASH "dev" CACHE STRING "git commit hash (should be set when making a release)")
if (PACKAGE_VERSION_COMMITHASH STREQUAL "dev")
    set(OPAQ_DEV_BUILD 1)
else ()
    set(OPAQ_DEV_BUILD 0)
endif ()

project(${PACKAGE}
    VERSION 0.5.0
    LANGUAGES CXX
)

include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "NSIS;ZIP;TGZ")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/dist")
set(CPACK_PACKAGE_NAME ${PACKAGE})
set(CPACK_PACKAGE_VENDOR "Vito")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_REVISION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_REVISION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Operational prediction of air quality")
set(CPACK_PACKAGE_EXECUTABLES "opaq;Opaq")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_SOURCE_GENERATOR "TGZ;TZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_SOURCE_STRIP_FILES "")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/ci/;/deps/;/dist/;/doc/;/run/;/.git/;/.*[.]sublime-.*;~$;${CPACK_SOURCE_IGNORE_FILES}")
set(CPACK_STRIP_FILES "opaq")

include(CPack)
include(GNUInstallDirs)

enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

option(OPAQ_SANITIZE_ADDRESS "Build with address sanitizer (detect invalid memory access and memory leaks)" OFF)
option(OPAQ_ENABLE_WERROR "Treat warnings as errors" OFF)
option(OPAQ_ENABLE_UI "Build the user interface" OFF)
option(OPAQ_STATIC_RUNTIME "Use static runtime on windows" OFF)
option(OPAQ_ENABLE_POSTGRESQL "Support postgresql in the sql plugin" OFF)
option(OPAQ_ENABLE_PREDICTION_SERVICE "Build the prediction REST service" OFF)

message(STATUS "Opaq ui enabled: ${OPAQ_ENABLE_UI}")

if (OPAQ_SANITIZE_ADDRESS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif ()

if (OPAQ_ENABLE_WERROR)
    if (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    endif ()
endif ()

if (MSVC)
    # avoid windows specific warnings
    add_definitions("-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS -D_SILENCE_FPOS_SEEKPOS_DEPRECATION_WARNING")
    # require windows7 service pack1
    add_definitions("-D_WIN32_WINNT=0x0601")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /w34100 /w34101 /w35038")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wfatal-errors -fdiagnostics-color")
endif()

if (MINGW)
    add_definitions("-DMINGW_HAS_SECURE_API")
endif ()

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
endif()

include_directories(
    ${CMAKE_BINARY_DIR}
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ${OPAQ_STATIC_RUNTIME})

set(INFRA_XML ON CACHE BOOL "" FORCE)
set(INFRA_LOGGING ON CACHE BOOL "" FORCE)
set(INFRA_GDAL ON CACHE BOOL "" FORCE)
set(INFRA_EMBED_GDAL_DATA ON CACHE BOOL "" FORCE)
set(INFRA_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(INFRA_UI_COMPONENTS ${OPAQ_ENABLE_UI} CACHE BOOL "" FORCE)
set(INFRA_UI_COMPONENTS_LOCATION ${OPAQ_ENABLE_UI} CACHE BOOL "" FORCE)
add_subdirectory(deps/infra)

set(GDX_DISABLE_OPENMP ON CACHE BOOL "" FORCE)
set(GDX_PYTHON_BINDINGS OFF CACHE BOOL "" FORCE)
set(GDX_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(GDX_ENABLE_TOOLS OFF CACHE BOOL "" FORCE)
set(GDX_PYTHON_BINDINGS OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/gdx)

add_subdirectory(src)

configure_file(${CMAKE_SOURCE_DIR}/opaqconfig.h.in ${CMAKE_BINARY_DIR}/opaqconfig.h)
if (WIN32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc @ONLY)
endif ()
