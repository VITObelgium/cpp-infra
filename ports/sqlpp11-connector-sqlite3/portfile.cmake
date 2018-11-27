set(VERSION_MAJOR 0)
set(VERSION_MINOR 29)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR})

include(vcpkg_common_functions)
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO rbock/${PORT}
    REF ${VERSION}
    SHA512 99d1b36209dc879b12b99ed0809f1d21f760c62c25aa32d8f83b571d0819e35783ad20be0754288da9cd5fcb81cbb672031928d159ff9a64c3635dcbc4bda8fa
    HEAD_REF master
    PATCHES
        ${CMAKE_CURRENT_LIST_DIR}/float-precision.patch
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DENABLE_TESTS=OFF
        -DSQLPP11_INCLUDE_DIR:FILEPATH=${CURRENT_INSTALLED_DIR}/include
)

vcpkg_install_cmake()
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

vcpkg_copy_pdbs()

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
