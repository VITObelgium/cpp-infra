include(vcpkg_common_functions)
vcpkg_from_git(
    URL https://github.com/mapbox/mapbox-gl-native.git
    OUT_SOURCE_PATH SOURCE_PATH
    REF b83030aa9bb1a8d9f14ae8160698c1ee4d5a4c72
    HEAD_REF master
    SHA512 5a3b093bf2b811449af20d512e88086a09d605c5f6a9ee4ba92db4a763c3091201413b23b7f9cdc3e687d6650b33b990f9383e314c0d748159804b4e52f652cc
)
    
vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}
    PATCHES
        c++17-support.patch
        use-vcpkg-deps.patch
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DVCPKG_ALLOW_SYSTEM_LIBS=ON
        -DWITH_NODEJS=OFF
        -DMBGL_PLATFORM=qt
		-DMASON_PLATFORM=$(MASON_PLATFORM)
		-DMASON_PLATFORM_VERSION=$(MASON_PLATFORM_VERSION)
		-DWITH_QT_DECODERS=OFF # use libjpeg-turbo and libpng from vcpkg
		-DWITH_QT_I18N=OFF # use libicu from vcpkg
        -DWITH_COVERAGE=OFF
        -DWITH_ERROR=OFF
)

vcpkg_build_cmake(TARGET qmapboxgl)

file(COPY ${SOURCE_PATH}/platform/qt/include DESTINATION ${CURRENT_PACKAGES_DIR})
file(COPY ${SOURCE_PATH}/include/mbgl DESTINATION ${CURRENT_PACKAGES_DIR}/include)
file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/lib)
file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/debug/lib)

if (MSVC)
    set(LIB_PREFIX "")
    set(LIB_EXT "lib")
else()
    set(LIB_PREFIX "lib")
    set(LIB_EXT "a")
endif()

set (MBGL_LIBS mbgl-core mbgl-filesource nunicode)
foreach(MBGL_LIB IN LISTS MBGL_LIBS)
    file(COPY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg/${LIB_PREFIX}${MBGL_LIB}d.${LIB_EXT} DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib)
    file(COPY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/${LIB_PREFIX}${MBGL_LIB}.${LIB_EXT} DESTINATION ${CURRENT_PACKAGES_DIR}/lib)
endforeach()

file(INSTALL ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg/${LIB_PREFIX}qmapboxgld.${LIB_EXT} DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib RENAME ${LIB_PREFIX}qmapboxgl-vcpkgd.${LIB_EXT})
file(INSTALL ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/${LIB_PREFIX}qmapboxgl.${LIB_EXT} DESTINATION ${CURRENT_PACKAGES_DIR}/lib RENAME ${LIB_PREFIX}qmapboxgl-vcpkg.${LIB_EXT})

file(INSTALL ${SOURCE_PATH}/LICENSE.md DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
