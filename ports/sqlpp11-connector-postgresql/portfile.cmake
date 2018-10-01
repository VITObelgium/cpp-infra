set(VERSION_MAJOR 0)
set(VERSION_MINOR 54)
set(VERSION v${VERSION_MAJOR}.${VERSION_MINOR})

include(vcpkg_common_functions)
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO matthijs/${PORT}
    REF ${VERSION}
    SHA512 0d26dc80e6e7d9f13e95c6aaf9b7550aa86298010e4ddd35af6cc43eed315da78eb343034691117583140be393da7af6e7c53b4e429db37023171159d79dcb7c
    HEAD_REF master
)

vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}
    PATCHES
    ${CMAKE_CURRENT_LIST_DIR}/disable-shared.patch
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DENABLE_TESTS=OFF
)

vcpkg_install_cmake()
vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/sqlpp-connector-postgresql)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/lib/cmake)

vcpkg_copy_pdbs()

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
