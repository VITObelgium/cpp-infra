include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO apache/xerces-c
    REF Xerces-C_3_2_2
    SHA512 66f60fe9194376ac0ca99d13ea5bce23ada86e0261dde30686c21ceb5499e754dab8eb0a98adadd83522bda62709377715501f6dac49763e3a686f9171cc63ea
    HEAD_REF trunk
    PATCHES
        disable-tests.patch
        transitive-icu.patch
        cmake-config-path.patch
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DDISABLE_TESTS=ON
        -DDISABLE_DOC=ON
        -DDISABLE_SAMPLES=ON
)

vcpkg_install_cmake()

vcpkg_fixup_cmake_targets(CONFIG_PATH share/XercesC TARGET_PATH share/${PORT})
vcpkg_fixup_pkgconfig_file(NAMES xerces-c)

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
)

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)

vcpkg_copy_pdbs()
vcpkg_test_cmake(PACKAGE_NAME XercesC)