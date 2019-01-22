include(vcpkg_common_functions)

set(MAJOR 3)
set(MINOR 0)
set(REVISION 22)
set(VERSION v${MAJOR}.${MINOR}.${REVISION})
set(PACKAGE_NAME ${PORT}-${VERSION})
set(PACKAGE ${PACKAGE_NAME}.tar.bz2)

set(Python_ADDITIONAL_VERSIONS 2.7 3.5)
find_package(PythonInterp REQUIRED)

# Extract source into architecture specific directory, because GDALs' build currently does not
# support out of source builds.
set(SOURCE_PATH_DEBUG   ${CURRENT_BUILDTREES_DIR}/src-${TARGET_TRIPLET}-debug/${PORT}-${VERSION})
set(SOURCE_PATH_RELEASE ${CURRENT_BUILDTREES_DIR}/src-${TARGET_TRIPLET}-release/${PORT}-${VERSION})

vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/${PORT}/${PORT}/releases/download/${VERSION}/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 a558d5e52b249b439d7c4b574d909d781c877d91d3b1e707938fb9856661542c2d7b36742c394e5a0a38d88cf9f65078093dcc4190e5d3a5672c0ef054eb6358
)

foreach(BUILD_TYPE debug release)
    set(CONFIG_SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src-${TARGET_TRIPLET}-${BUILD_TYPE})
    file(REMOVE_RECURSE ${CONFIG_SOURCE_PATH})
    vcpkg_extract_source_archive(${ARCHIVE} ${CONFIG_SOURCE_PATH})
    vcpkg_apply_patches(
        SOURCE_PATH ${CONFIG_SOURCE_PATH}/${PACKAGE_NAME}
        PATCHES
        ${CMAKE_CURRENT_LIST_DIR}/mapnik-render-link.patch
        ${CMAKE_CURRENT_LIST_DIR}/tiff-link.patch
        ${CMAKE_CURRENT_LIST_DIR}/config-path.patch
        ${CMAKE_CURRENT_LIST_DIR}/gdallib-detection.patch
    )
endforeach()

if (VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
    include (${VCPKG_CHAINLOAD_TOOLCHAIN_FILE})
elseif(VCPKG_TARGET_TRIPLET)
    # include the triplet cmake toolchain file if it is present unless a chainload toolchain was provided
    include(${CMAKE_CURRENT_LIST_DIR}/../../triplets/${VCPKG_TARGET_TRIPLET}.cmake OPTIONAL)
    message(STATUS "XXX using triplet toolchain settings ${CMAKE_CURRENT_LIST_DIR}/../../triplets/${VCPKG_TARGET_TRIPLET}.cmake")
endif ()

set(MAPNIK_INPUT_PLUGINS "raster,shape,csv,geojson")

set(SCONS_OPTIONS
    CUSTOM_CFLAGS=${CMAKE_C_FLAGS}
    CC=${CMAKE_C_COMPILER}
    CXX=${CMAKE_CXX_COMPILER}
    LD=${LINKER_EXECUTABLE}
    AR=${CMAKE_AR}
    RANLIB=${CMAKE_RANLIB}
    PATH=${CURRENT_INSTALLED_DIR}/tools
    CUSTOM_LDFLAGS=${CMAKE_EXE_LINKER_FLAGS}
    BOOST_INCLUDES=${CURRENT_INSTALLED_DIR}/include
    ICU_INCLUDES=${CURRENT_INSTALLED_DIR}/include
    FREETYPE_INCLUDES=${CURRENT_INSTALLED_DIR}/include/freetype2
    HB_INCLUDES=${CURRENT_INSTALLED_DIR}/include
    PROJ_INCLUDES=${CURRENT_INSTALLED_DIR}/include
    PNG_INCLUDES=${CURRENT_INSTALLED_DIR}/include
    TIFF_INCLUDES=${CURRENT_INSTALLED_DIR}/include
    CAIRO=no
    JPEG=no
    WEBP=no
    DEMO=no
    CPP_TESTS=no
    BINDINGS=none
    SHAPEINDEX=no
    MAPNIK_INDEX=no
    PATH_REMOVE=/usr
)

if (${VCPKG_LIBRARY_LINKAGE} STREQUAL "static")
    list(APPEND SCONS_OPTIONS LINKING=static)
    list(APPEND SCONS_OPTIONS PLUGIN_LINKING=static)
else ()
    list(APPEND SCONS_OPTIONS LINKING=shared)
    list(APPEND SCONS_OPTIONS PLUGIN_LINKING=shared)
endif ()

if (${VCPKG_CRT_LINKAGE} STREQUAL "static")
    list(APPEND SCONS_OPTIONS RUNTIME_LINK=static)
else ()
    list(APPEND SCONS_OPTIONS RUNTIME_LINK=shared)
endif ()

if ("gdal" IN_LIST FEATURES)
    list(APPEND SCONS_OPTIONS GDAL_CONFIG=${CURRENT_INSTALLED_DIR}/tools/gdal-config)
    set(MAPNIK_INPUT_PLUGINS "${MAPNIK_INPUT_PLUGINS},gdal")
else ()
    list(APPEND SCONS_OPTIONS GDAL_CONFIG=${CURRENT_INSTALLED_DIR}/tools/invalid-config)
endif ()

list(APPEND SCONS_OPTIONS INPUT_PLUGINS=${MAPNIK_INPUT_PLUGINS})
vcpkg_assemble_compiler_cxxflags(DEBUG MAPNIK_CXX_FLAGS_DEBUG RELEASE MAPNIK_CXX_FLAGS_RELEASE)

set(SCONS_OPTIONS_REL
    CUSTOM_CXXFLAGS=${MAPNIK_CXX_FLAGS_RELEASE}
    ${SCONS_OPTIONS}
    DEBUG=no
    PREFIX=${CURRENT_PACKAGES_DIR}
    BOOST_LIBS=${CURRENT_INSTALLED_DIR}/lib
    ICU_LIBS=${CURRENT_INSTALLED_DIR}/lib
    FREETYPE_LIBS=${CURRENT_INSTALLED_DIR}/lib
    HB_LIBS=${CURRENT_INSTALLED_DIR}/lib
    PROJ_LIBS=${CURRENT_INSTALLED_DIR}/lib
    PNG_LIBS=${CURRENT_INSTALLED_DIR}/lib
    TIFF_LIBS=${CURRENT_INSTALLED_DIR}/lib
)

set(SCONS_OPTIONS_DBG
    ${SCONS_OPTIONS}
    DEBUG=yes
    CUSTOM_CXXFLAGS=${MAPNIK_CXX_FLAGS_DEBUG}
    PREFIX=${CURRENT_PACKAGES_DIR}/debug
    BOOST_LIBS=${CURRENT_INSTALLED_DIR}/debug/lib
    ICU_LIBS=${CURRENT_INSTALLED_DIR}/debug/lib
    FREETYPE_LIBS=${CURRENT_INSTALLED_DIR}/debug/lib
    HB_LIBS=${CURRENT_INSTALLED_DIR}/debug/lib
    PROJ_LIBS=${CURRENT_INSTALLED_DIR}/lib
    PNG_LIBS=${CURRENT_INSTALLED_DIR}/debug/lib
    TIFF_LIBS=${CURRENT_INSTALLED_DIR}/debug/lib
)

message(STATUS "Configuring ${TARGET_TRIPLET}-rel")
message(STATUS "${SCONS_OPTIONS_REL}")
vcpkg_execute_required_process(
    COMMAND ${PYTHON_EXECUTABLE} scons/scons.py configure
    "${SCONS_OPTIONS_REL}"
    WORKING_DIRECTORY ${SOURCE_PATH_RELEASE}
    LOGNAME scons-configure-${TARGET_TRIPLET}-release
)

message(STATUS "Configuring ${TARGET_TRIPLET}-dbg")
vcpkg_execute_required_process(
    COMMAND ${PYTHON_EXECUTABLE} scons/scons.py configure
    "${SCONS_OPTIONS_DBG}"
    WORKING_DIRECTORY ${SOURCE_PATH_DEBUG}
    LOGNAME scons-configure-${TARGET_TRIPLET}-debug
)

include(ProcessorCount)
ProcessorCount(NUM_CORES)

message(STATUS "Building ${TARGET_TRIPLET}-rel")
vcpkg_execute_required_process(
    COMMAND ${PYTHON_EXECUTABLE} scons/scons.py --jobs=${NUM_CORES}
    WORKING_DIRECTORY ${SOURCE_PATH_RELEASE}
    LOGNAME scons-build-${TARGET_TRIPLET}-release
)

message(STATUS "Building ${TARGET_TRIPLET}-dbg")
vcpkg_execute_required_process(
    COMMAND ${PYTHON_EXECUTABLE} scons/scons.py --jobs=${NUM_CORES}
    WORKING_DIRECTORY ${SOURCE_PATH_DEBUG}
    LOGNAME scons-build-${TARGET_TRIPLET}-debug
)

message(STATUS "Installing ${TARGET_TRIPLET}-rel")
vcpkg_execute_required_process(
    COMMAND ${PYTHON_EXECUTABLE} scons/scons.py install
    WORKING_DIRECTORY ${SOURCE_PATH_RELEASE}
    LOGNAME scons-install-${TARGET_TRIPLET}-release
)

message(STATUS "Installing ${TARGET_TRIPLET}-dbg")
vcpkg_execute_required_process(
    COMMAND ${PYTHON_EXECUTABLE} scons/scons.py install
    WORKING_DIRECTORY ${SOURCE_PATH_DEBUG}
    LOGNAME scons-install-${TARGET_TRIPLET}-debug
)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/bin)

file(GLOB BIN_FILES ${CURRENT_PACKAGES_DIR}/bin/*)
file(COPY ${BIN_FILES} DESTINATION ${CURRENT_PACKAGES_DIR}/tools)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/bin)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/lib/mapnik)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/lib/mapnik)

# Handle copyright
file(INSTALL ${SOURCE_PATH_RELEASE}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
