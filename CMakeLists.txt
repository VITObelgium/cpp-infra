cmake_minimum_required(VERSION 3.9)
cmake_policy(VERSION 3.9)
cmake_policy(SET CMP0071 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/deps/cmake-scripts/scripts")
include(FindDependency)

set(PACKAGE "opaq")
set(PACKAGE_VERSION_MAJOR 0)
set(PACKAGE_VERSION_MINOR 5)
set(PACKAGE_VERSION_REVISION 0)

project(${PACKAGE})

include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "NSIS;ZIP;TGZ")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/dist")
set(CPACK_PACKAGE_NAME ${PACKAGE})
set(CPACK_PACKAGE_VENDOR "Vito")
set(CPACK_PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_REVISION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PACKAGE_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PACKAGE_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PACKAGE_VERSION_REVISION}")
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
option(OPAQ_ENABLE_POSTGRESQL "Support postgresql in the sql plugin" OFF)
option(OPAQ_ENABLE_PREDICTION_SERVICE "Build the prediction REST service" OFF)

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
    add_definitions("-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /w34100 /w34101 /w35038")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wfatal-errors -fdiagnostics-color")
endif()

if (MINGW)
    add_definitions("-DMINGW_HAS_SECURE_API")
endif ()

include_directories(
    ${CMAKE_BINARY_DIR}
)

set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(INFRA_XML ON CACHE BOOL "" FORCE)
set(INFRA_GDAL OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/infra)
add_subdirectory(src)

configure_file(${CMAKE_SOURCE_DIR}/opaqconfig.h.in ${CMAKE_BINARY_DIR}/opaqconfig.h)
