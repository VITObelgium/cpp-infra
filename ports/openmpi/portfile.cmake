include(vcpkg_common_functions)
set(VERSION_MAJOR 3)
set(VERSION_MINOR 1)
set(VERSION_REVISION 2)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION})
set(PACKAGE_NAME ${PORT}-${VERSION})
set(PACKAGE ${PACKAGE_NAME}.tar.bz2)

vcpkg_download_distfile(ARCHIVE
    URLS "https://download.open-mpi.org/release/open-mpi/v${VERSION_MAJOR}.${VERSION_MINOR}/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 ec8df8e0ac89f5573adfd25707a03a583069012a3a874c939ede71635198045565e5e9ddf0181cea474a1a6baaf8d7ba647e2ed194d1b29a1882c1fc18967b57
)
vcpkg_extract_source_archive(${ARCHIVE})

set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/${PACKAGE_NAME})

if (UNIX AND NOT APPLE)
    set(VCPKG_BUILD_TYPE release)

    vcpkg_configure_autoconf(
        SOURCE_PATH ${SOURCE_PATH}
        OPTIONS
            --bindir=${CURRENT_PACKAGES_DIR}/tools
            --disable-java
            --disable-visibility # handled by the compiler flags
    )

    vcpkg_build_autotools()
    vcpkg_install_autotools()
    vcpkg_fixup_pkgconfig_file(NAMES ompi-c ompi-cxx ompi-f90 ompi-fort ompi orte)
else ()
    message(FATAL_ERROR "${PORT} is only supported on unix")
endif()

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
