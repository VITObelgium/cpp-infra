include(vcpkg_common_functions)

if (EXISTS ${CURRENT_INSTALLED_DIR}/lib/libqmapboxgl.a AND 
    EXISTS ${CURRENT_INSTALLED_DIR}/lib/libqmapboxgl.prl)
    message(FATAL_ERROR "The qmapboxql library is already installed as part of qt")
endif ()

vcpkg_from_git(
    URL https://github.com/dirkvdb/mapbox-gl-native.git
    OUT_SOURCE_PATH SOURCE_PATH
    REF vcpkg
    HEAD_REF master
    SHA512 0
    RECURSE_SUBMODULES
)
    
TEST_FEATURE("qtplugin" WITH_QTPLUGIN)

if((VCPKG_CMAKE_SYSTEM_NAME STREQUAL "WindowsStore" OR NOT DEFINED VCPKG_CMAKE_SYSTEM_NAME) AND NOT MINGW)
    set(GENERATOR "Visual Studio 15 2017 Win64")
    set(ADDITIONAL_ARGS -T LLVM-vs2017)
else ()
    set(GENERATOR "Ninja")
endif()

string(COMPARE EQUAL "static" "${VCPKG_LIBRARY_LINKAGE}" STATIC_QT)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    GENERATOR ${GENERATOR}
    OPTIONS
        -DVCPKG_ALLOW_SYSTEM_LIBS=ON
        -DWITH_NODEJS=OFF
        -DMBGL_PLATFORM=qt
        -DWITH_QT_DECODERS=OFF # use libjpeg-turbo and libpng from vcpkg
        -DWITH_QT_I18N=OFF # use libicu from vcpkg
        -DWITH_QT_GEOPLUGIN=${WITH_QTPLUGIN}
        -DWITH_COVERAGE=OFF
        -DWITH_ERROR=OFF
        -DSTATIC_QT=${STATIC_QT}
        ${ADDITIONAL_ARGS}
)

vcpkg_install_cmake()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

file(INSTALL ${CMAKE_CURRENT_LIST_DIR}/FindMapboxGL.cmake DESTINATION ${CURRENT_PACKAGES_DIR}/share/cmake)
file(INSTALL ${SOURCE_PATH}/LICENSE.md DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
