include(vcpkg_common_functions)
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO gabime/spdlog
    REF v1.2.1
    SHA512 418f91efc207fa227558212d82c41639c0bb59e84ea47447e0b6276c4842e97f1f8aaf5802c071ef15d80ec525e317e70b6a39661a6c96ab39d33d9bd1570da1
    HEAD_REF master
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DSPDLOG_BUILD_TESTING=OFF
)

vcpkg_install_cmake()

# Move cmake files, ensuring they will be 3 directories up the import prefix
file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/share/spdlog)
file(RENAME ${CURRENT_PACKAGES_DIR}/lib/cmake/spdlog/ ${CURRENT_PACKAGES_DIR}/share/spdlog/cmake)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/lib)

# Handle copyright
file(COPY ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/spdlog)
file(RENAME ${CURRENT_PACKAGES_DIR}/share/spdlog/LICENSE ${CURRENT_PACKAGES_DIR}/share/spdlog/copyright)
