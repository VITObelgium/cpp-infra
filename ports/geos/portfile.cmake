set(VERSION_MAJOR 3)
set(VERSION_MINOR 7)
set(VERSION_REVISION 1)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION})
set(PACKAGE_NAME ${PORT}-${VERSION})
set(PACKAGE ${PACKAGE_NAME}.tar.bz2)

include(vcpkg_common_functions)
set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/${PACKAGE_NAME})

vcpkg_download_distfile(ARCHIVE
    URLS "http://download.osgeo.org/geos/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 01e8087bcd3cb8f873adb7b56910e1575ccb3336badfdd3f13bc6792095b7010e5ab109ea0d0cd3d1459e2e526e83bcf64d6ee3f7eb47be75639becdaacd2a87
)
vcpkg_extract_source_archive(${ARCHIVE})

if (UNIX)
    # cmake build system is not stable on linux
    vcpkg_configure_autoconf(
        SOURCE_PATH ${SOURCE_PATH}
        CXX_STANDARD 14
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
    string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" GEOS_STATIC)
    string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "shared" GEOS_SHARED)

    vcpkg_configure_cmake(
        SOURCE_PATH ${SOURCE_PATH}
        PREFER_NINJA # Disable this option if project cannot be built with Ninja
        OPTIONS
        -DGEOS_ENABLE_TESTS=OFF
        -DGEOS_BUILD_STATIC=${GEOS_STATIC}
        -DGEOS_BUILD_SHARED=${GEOS_SHARED}
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
