set(VERSION_MAJOR 3)
set(VERSION_MINOR 6)
set(VERSION_REVISION 3)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION})
set(PACKAGE_NAME ${PORT}-${VERSION})
set(PACKAGE ${PACKAGE_NAME}.tar.bz2)

include(vcpkg_common_functions)
set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/${PACKAGE_NAME})

vcpkg_download_distfile(ARCHIVE
    URLS "http://download.osgeo.org/geos/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 f88adcf363433e247a51fb1a2c0b53f39b71aba8a6c01dd08aa416c2e980fe274a195e6edcb5bb5ff8ea81b889da14a1a8fb2849e04669aeba3b6d55754dc96a
)
vcpkg_extract_source_archive(${ARCHIVE})

if (UNIX)
    # cmake build system is not stable on linux
    vcpkg_configure_autoconf(
        SOURCE_PATH ${SOURCE_PATH}
        OPTIONS
            --enable-python=no
            --enable-ruby=no
    )

    vcpkg_install_autotools()
else ()
    vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}
        PATCHES ${CMAKE_CURRENT_LIST_DIR}/geos_c-static-support.patch
    )

    # NOTE: GEOS provides CMake as optional build configuration, it might not be actively
    # maintained, so CMake build issues may happen between releases.

    # Pull modules referred in the main CMakeLists.txt but missing from the released package.
    # TODO: GEOS 3.6.3 or later will include the missing script in release package.
    file(DOWNLOAD http://svn.osgeo.org/geos/branches/3.6/cmake/modules/GenerateSourceGroups.cmake
        ${SOURCE_PATH}/cmake/modules/GenerateSourceGroups.cmake)

    vcpkg_configure_cmake(
        SOURCE_PATH ${SOURCE_PATH}
        PREFER_NINJA # Disable this option if project cannot be built with Ninja
        OPTIONS
        -DGEOS_ENABLE_TESTS=False
    )
    vcpkg_install_cmake()
endif ()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)

if (UNIX)
    # move the geos-config script to the the tools directory on unix
    vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/bin/geos-config "packages/${PORT}_${TARGET_TRIPLET}" "installed/${TARGET_TRIPLET}")
    file(COPY ${CURRENT_PACKAGES_DIR}/bin/geos-config DESTINATION ${CURRENT_PACKAGES_DIR}/tools)
else ()
    if(VCPKG_LIBRARY_LINKAGE STREQUAL dynamic)
        file(REMOVE ${CURRENT_PACKAGES_DIR}/lib/libgeos.lib ${CURRENT_PACKAGES_DIR}/debug/lib/libgeosd.lib)
    else()
        file(REMOVE ${CURRENT_PACKAGES_DIR}/lib/geos.lib ${CURRENT_PACKAGES_DIR}/debug/lib/geosd.lib)
        file(REMOVE ${CURRENT_PACKAGES_DIR}/lib/geos_c.lib ${CURRENT_PACKAGES_DIR}/debug/lib/geos_cd.lib)
    endif()

    vcpkg_copy_pdbs()
endif ()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/bin ${CURRENT_PACKAGES_DIR}/debug/bin)
file(INSTALL ${CMAKE_CURRENT_LIST_DIR}/FindGeos.cmake DESTINATION ${CURRENT_PACKAGES_DIR}/share/cmake)
