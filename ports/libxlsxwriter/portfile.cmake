include(vcpkg_common_functions)

set(MAJOR 0)
set(MINOR 7)
set(REVISION 9)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(SHA_SUM 935fa66c5e6c1b5dc07ee5f56a485b42e7505fd270215998d2503beb85c142a3c77abf0eb9d9972d34a0671a66cd4fa9b3cf03edd7742098a8fdb1f5e598fe88)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO jmcnamara/libxlsxwriter
    REF RELEASE_${VERSION}
    SHA512 ${SHA_SUM}
    HEAD_REF master
    PATCHES ${CMAKE_CURRENT_LIST_DIR}/fix-msvc-install.patch
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DBUILD_TESTS=OFF
        -DBUILD_EXAMPLES=OFF
)

vcpkg_install_cmake()
vcpkg_copy_pdbs()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(INSTALL ${CMAKE_CURRENT_LIST_DIR}/FindXlsxWriter.cmake DESTINATION ${CURRENT_PACKAGES_DIR}/share/cmake)
file(INSTALL ${SOURCE_PATH}/License.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)