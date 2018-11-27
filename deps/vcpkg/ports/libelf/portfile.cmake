include(vcpkg_common_functions)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 8)
set(VERSION_REVISION 13)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION})
set(PACKAGE_NAME ${PORT}-${VERSION})
set(PACKAGE ${PACKAGE_NAME}.tar.gz)

vcpkg_download_distfile(ARCHIVE
    URLS "http://www.mr511.de/software/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 d2a4ea8ccc0bbfecac38fa20fbd96aefa8e86f8af38691fb6991cd9c5a03f587475ecc2365fc89a4954c11a679d93460ee9a5890693112f6133719af3e6582fe
)
vcpkg_extract_source_archive(${ARCHIVE})

set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/${PACKAGE_NAME})

if (UNIX AND NOT APPLE)
    set(VCPKG_BUILD_TYPE release)

    vcpkg_configure_autoconf(
        SOURCE_PATH ${SOURCE_PATH}
    )

    vcpkg_build_autotools()
    vcpkg_install_autotools()
    vcpkg_fixup_pkgconfig_file()
else ()
    message(FATAL_ERROR "${PORT} is only supported on unix")
endif()

# Handle copyright
file(INSTALL ${SOURCE_PATH}/COPYING.LIB DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
