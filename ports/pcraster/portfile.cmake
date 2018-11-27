include(vcpkg_common_functions)
set(MAJOR 4)
set(MINOR 2)
set(REVISION 0)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(PACKAGE_NAME ${PORT}-${VERSION})
set(PACKAGE ${PORT}-${VERSION}.tar.bz2)

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)
set(VCPKG_BUILD_TYPE release)
set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/${PACKAGE_NAME})

vcpkg_download_distfile(ARCHIVE
    URLS "http://pcraster.geo.uu.nl/${PORT}/${VERSION}/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 c3c836ff2e0cf837f41964d8670301b7b75975326b51aee76f9efed529de877e4e23ac4b77b2f0b09936aff6e8d6dd6e65a86565921b2609b598e0ab7ed8075e
)
vcpkg_extract_source_archive(${ARCHIVE})
vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}
    PATCHES minimize-build.patch
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DPCRASTER_BUILD_DOCUMENTATION=OFF
        -DPCRASTER_BUILD_TEST=OFF
        -DPCRASTER_WITH_AGUILA=OFF
        -DPCRASTER_WITH_MODFLOW=ON
        -DFERN_BUILD_ALGORITHM=TRUE
        -DPYTHON_EXECUTABLE=/projects/urbflood/.miniconda3/bin/python
        -DPYTHON_INCLUDE_DIRS=/projects/urbflood/.miniconda3/include
        -DPYTHON_LIBRARY=/projects/urbflood/.miniconda3/lib/libpython3.7m.so
)

vcpkg_install_cmake()
vcpkg_copy_pdbs()

file(GLOB PCRASTER_BINARIES ${CURRENT_PACKAGES_DIR}/bin/*)
file(COPY ${PCRASTER_BINARIES} DESTINATION ${CURRENT_PACKAGES_DIR}/tools/)

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/bin"
    "${CURRENT_PACKAGES_DIR}/doc"
    "${CURRENT_PACKAGES_DIR}/lib"
    "${CURRENT_PACKAGES_DIR}/include"
    "${CURRENT_PACKAGES_DIR}/share/gdal"
)

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
