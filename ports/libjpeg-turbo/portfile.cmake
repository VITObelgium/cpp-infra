include(vcpkg_common_functions)
set(MAJOR 1)
set(MINOR 5)
set(REVISION 3)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(SHA512_HASH 0e7a2cd9943b610f49b562c20a5c350a50326a87bce1d39f14fe45760ed2f89a0d2d3e3f0de9f6a7714f566aabadec6b2422b592591ebb98bbad600ea411fea7)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO ${PORT}/${PORT}
    REF ${VERSION}
    SHA512 ${SHA512_HASH}
    HEAD_REF master
)

vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}
    PATCHES "${CMAKE_CURRENT_LIST_DIR}/add-options-for-exes-docs-headers.patch"
        "${CMAKE_CURRENT_LIST_DIR}/linux-cmake.patch"
)

if(VCPKG_TARGET_ARCHITECTURE STREQUAL "arm" OR VCPKG_TARGET_ARCHITECTURE STREQUAL "arm64" OR (VCPKG_CMAKE_SYSTEM_NAME AND NOT VCPKG_CMAKE_SYSTEM_NAME STREQUAL "WindowsStore"))
    set(LIBJPEGTURBO_SIMD -DWITH_SIMD=OFF)
else()
    set(LIBJPEGTURBO_SIMD -DWITH_SIMD=ON)
    vcpkg_find_acquire_program(NASM)
    get_filename_component(NASM_EXE_PATH ${NASM} DIRECTORY)
    set(ENV{PATH} "$ENV{PATH};${NASM_EXE_PATH}")
endif()

if(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
    set(ENV{_CL_} "-DNO_GETENV -DNO_PUTENV")
endif()

string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "dynamic" ENABLE_SHARED)
string(COMPARE EQUAL "${VCPKG_LIBRARY_LINKAGE}" "static" ENABLE_STATIC)
string(COMPARE EQUAL "${VCPKG_CRT_LINKAGE}" "dynamic" WITH_CRT_DLL)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DENABLE_STATIC=${ENABLE_STATIC}
        -DENABLE_SHARED=${ENABLE_SHARED}
        -DENABLE_EXECUTABLES=OFF
        -DINSTALL_DOCS=OFF
        -DWITH_CRT_DLL=${WITH_CRT_DLL}
        ${LIBJPEGTURBO_SIMD}
    OPTIONS_DEBUG -DINSTALL_HEADERS=OFF
)

vcpkg_install_cmake()
vcpkg_copy_pdbs()

# Rename libraries for static builds
if(VCPKG_LIBRARY_LINKAGE STREQUAL "static" AND EXISTS "${CURRENT_PACKAGES_DIR}/lib/jpeg-static.lib")
    file(RENAME "${CURRENT_PACKAGES_DIR}/lib/jpeg-static.lib" "${CURRENT_PACKAGES_DIR}/lib/jpeg.lib")
    file(RENAME "${CURRENT_PACKAGES_DIR}/lib/turbojpeg-static.lib" "${CURRENT_PACKAGES_DIR}/lib/turbojpeg.lib")
    file(RENAME "${CURRENT_PACKAGES_DIR}/debug/lib/jpeg-static.lib" "${CURRENT_PACKAGES_DIR}/debug/lib/jpeg.lib")
    file(RENAME "${CURRENT_PACKAGES_DIR}/debug/lib/turbojpeg-static.lib" "${CURRENT_PACKAGES_DIR}/debug/lib/turbojpeg.lib")
endif()

file(INSTALL ${SOURCE_PATH}/LICENSE.md DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/usage DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT})
