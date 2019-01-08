include(vcpkg_common_functions)
set(MAJOR 2)
set(MINOR 8)
set(REVISION 2)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(PACKAGE ${PORT}-${VERSION}.tar.gz)
set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/${PORT}-${VERSION})
vcpkg_download_distfile(ARCHIVE_FILE
    URLS "http://ftp.openbsd.org/pub/OpenBSD/LibreSSL/${PACKAGE}" "http://mirror.nl.datapacket.com/openbsd/LibreSSL/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 bfcc55904efbb591c9edd56169d611e735108dfc6a49f771a64ad1ddd028d3a658f5593116c379911edc77f95eba475daec9c0adea0549e8b4b94d1072adf733
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
