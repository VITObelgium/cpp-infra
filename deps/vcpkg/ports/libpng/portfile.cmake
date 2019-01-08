include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO glennrp/libpng
    REF v1.6.36
    SHA512 aeb00b48347c9e84d31995b3fe7e40580029734aa8103d774eee5745f5ca1fd1fd91a15f32d492277ab94346e4e7f731ee9bfea1783f930094f9f87eb3d9397d
    HEAD_REF master
)

if(VCPKG_LIBRARY_LINKAGE STREQUAL dynamic)
    set(PNG_STATIC_LIBS OFF)
    set(PNG_SHARED_LIBS ON)
else()
    set(PNG_STATIC_LIBS ON)
    set(PNG_SHARED_LIBS OFF)
endif()

if (VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Linux" OR VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    # finding the math library fails on some toolchains
    # on apple libm is not presen on disk, so detection fails
    set(EXTRA_OPTIONS -DM_LIBRARY=m)
endif ()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DPNG_STATIC=${PNG_STATIC_LIBS}
        -DPNG_SHARED=${PNG_SHARED_LIBS}
        -DPNG_TESTS=OFF
        -DSKIP_INSTALL_PROGRAMS=ON
        -DSKIP_INSTALL_EXECUTABLES=ON
        -DSKIP_INSTALL_FILES=ON
        -DSKIP_INSTALL_SYMLINK=ON
        ${EXTRA_OPTIONS}
    OPTIONS_DEBUG
        -DSKIP_INSTALL_HEADERS=ON
)

vcpkg_install_cmake()

if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    if(EXISTS ${CURRENT_PACKAGES_DIR}/lib/libpng16_static.lib)
        file(RENAME ${CURRENT_PACKAGES_DIR}/lib/libpng16_static.lib ${CURRENT_PACKAGES_DIR}/lib/libpng.lib)
    endif()
    if(EXISTS ${CURRENT_PACKAGES_DIR}/debug/lib/libpng16_staticd.lib)
        file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/libpng16_staticd.lib ${CURRENT_PACKAGES_DIR}/debug/lib/libpngd.lib)
    endif()

    if(EXISTS ${CURRENT_PACKAGES_DIR}/lib/libpng16.a)
        file(RENAME ${CURRENT_PACKAGES_DIR}/lib/libpng16.a ${CURRENT_PACKAGES_DIR}/lib/libpng.a)
    endif()
    if(EXISTS ${CURRENT_PACKAGES_DIR}/debug/lib/libpng16d.a)
        file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/libpng16d.a ${CURRENT_PACKAGES_DIR}/debug/lib/libpngd.a)
    endif()

    file(REMOVE ${CURRENT_PACKAGES_DIR}/debug/lib/libpng.a)
endif()

# Remove CMake config files as they are incorrectly generated and everyone uses built-in FindPNG anyway.
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/lib/libpng ${CURRENT_PACKAGES_DIR}/debug/lib/libpng)
file(COPY ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/libpng)
file(RENAME ${CURRENT_PACKAGES_DIR}/share/libpng/LICENSE ${CURRENT_PACKAGES_DIR}/share/libpng/copyright)

vcpkg_copy_pdbs()

file(COPY ${CMAKE_CURRENT_LIST_DIR}/usage DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT})
