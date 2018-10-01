include(vcpkg_common_functions)

set(LIBTIFF_VERSION 4.0.9)
set(LIBTIFF_HASH 04f3d5eefccf9c1a0393659fe27f3dddd31108c401ba0dc587bca152a1c1f6bc844ba41622ff5572da8cc278593eff8c402b44e7af0a0090e91d326c2d79f6cd)
set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/tiff-${LIBTIFF_VERSION})

vcpkg_download_distfile(ARCHIVE
    URLS "http://download.osgeo.org/libtiff/tiff-${LIBTIFF_VERSION}.tar.gz"
    FILENAME "tiff-${LIBTIFF_VERSION}.tar.gz"
    SHA512 ${LIBTIFF_HASH}
)
vcpkg_extract_source_archive(${ARCHIVE})

vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}
    PATCHES ${CMAKE_CURRENT_LIST_DIR}/add-component-options.patch
            ${CMAKE_CURRENT_LIST_DIR}/fix-cxx-shared-libs.patch
            ${CMAKE_CURRENT_LIST_DIR}/crt-secure-no-deprecate.patch
)

if(VCPKG_TARGET_ARCHITECTURE STREQUAL "arm64" OR VCPKG_TARGET_ARCHITECTURE STREQUAL "arm")
    set (TIFF_CXX_TARGET -Dcxx=OFF)
endif()

set(TIFF_JPEG_SUPPORT OFF)
if("jpeg" IN_LIST FEATURES)
  set(TIFF_JPEG_SUPPORT ON)
endif()

set(TIFF_LZW_SUPPORT OFF)
if("lzw" IN_LIST FEATURES)
  set(TIFF_LZW_SUPPORT ON)
endif()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DBUILD_TOOLS=OFF
        -DBUILD_DOCS=OFF
        -DBUILD_CONTRIB=OFF
        -DBUILD_TESTS=OFF
        -Dlzma=${TIFF_LZW_SUPPORT}
        -Djpeg=${TIFF_JPEG_SUPPORT}
        -Djbig=OFF # This is disabled by default due to GPL/Proprietary licensing.
        -Djpeg12=OFF
        ${TIFF_CXX_TARGET}
)

vcpkg_install_cmake()

file(REMOVE_RECURSE
    ${CURRENT_PACKAGES_DIR}/debug/include
    ${CURRENT_PACKAGES_DIR}/debug/share
    ${CURRENT_PACKAGES_DIR}/share
)
file(INSTALL
    ${SOURCE_PATH}/COPYRIGHT
    DESTINATION ${CURRENT_PACKAGES_DIR}/share/tiff
    RENAME copyright
)

vcpkg_copy_pdbs()
vcpkg_fixup_pkgconfig_file(NAMES libtiff-4)
