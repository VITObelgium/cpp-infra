include(vcpkg_common_functions)
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO gabime/spdlog
    REF v1.3.0
    SHA512 019a52d4b6c66287ee2a6e8177457ecbbb78e1cb894f4a0a90b83a84d66cd37b397cdf77892d9116e4c34113bd3277d606d578bc96ec6521ae7745f08b1aa54f
    HEAD_REF master
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DSPDLOG_BUILD_TESTS=OFF
        -DSPDLOG_BUILD_BENCH=OFF
        -DSPDLOG_BUILD_EXAMPLES=OFF
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
