include(vcpkg_common_functions)
set(MAJOR 2)
set(MINOR 8)
set(REVISION 1)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(PACKAGE ${PORT}-${VERSION}.tar.gz)
set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/${PORT}-${VERSION})
vcpkg_download_distfile(ARCHIVE_FILE
    URLS "http://ftp.openbsd.org/pub/OpenBSD/LibreSSL/${PACKAGE}" "http://mirror.nl.datapacket.com/openbsd/LibreSSL/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 57af2c7a1a8522dca25c4e6371cb44f5ab074be1aded153e6e5fca4fa0844518710f7ce834d4dd309086686c492f10fca83f4d45c084eb49607cb5861f07ac99
)
vcpkg_extract_source_archive(${ARCHIVE_FILE})

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DLIBRESSL_TESTS=OFF
        -DLIBRESSL_APPS=OFF
)

vcpkg_install_cmake()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/share/man)

file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)

vcpkg_copy_pdbs()
vcpkg_fixup_pkgconfig_file(NAMES libcrypto libssl libtls openssl)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/usage DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT})
