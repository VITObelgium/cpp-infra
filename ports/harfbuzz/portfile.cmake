include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO harfbuzz/harfbuzz
    REF 2.1.3
    SHA512 45eef4ef4df1ed76cce755f544f6d6011e76cd25281783f8af921749924b894e49445e730249f4abf64bd1355cac8ce70b356793af8fb33c9e1a9fda6529dd19
    HEAD_REF master
)

vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}
    PATCHES
        "${CMAKE_CURRENT_LIST_DIR}/find-package-freetype-2.patch"
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DHB_HAVE_FREETYPE=ON
        -DHB_HAVE_ICU=ON
        -DHB_BUILD_UTILS=OFF
        -DHB_HAVE_GOBJECT=OFF
        -DHB_HAVE_CORETEXT=OFF
    OPTIONS_DEBUG
        -DSKIP_INSTALL_HEADERS=ON
)

vcpkg_install_cmake()

vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/harfbuzz)
vcpkg_copy_pdbs()

# Handle copyright
file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/harfbuzz RENAME copyright)
