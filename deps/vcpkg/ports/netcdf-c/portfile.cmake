# Common Ambient Variables:
#   VCPKG_ROOT_DIR = <C:\path\to\current\vcpkg>
#   TARGET_TRIPLET is the current triplet (x86-windows, etc)
#   PORT is the current port name (zlib, etc)
#   CURRENT_BUILDTREES_DIR = ${VCPKG_ROOT_DIR}\buildtrees\${PORT}
#   CURRENT_PACKAGES_DIR  = ${VCPKG_ROOT_DIR}\packages\${PORT}_${TARGET_TRIPLET}
#

include(vcpkg_common_functions)
set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/netcdf-c-4.4.1.1)
vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/Unidata/netcdf-c/archive/v4.4.1.1.zip"
    FILENAME "netcdf-c-v4.4.1.1.zip"
    SHA512 8d31e47f0bd4d5c8640d3444c5c83425862ed1e8283aa78419e86b18d33fd93bfeb0067e8e9630457cb9c334d6fedf630283b5227a15137a3e153d57b22364a8
)
vcpkg_extract_source_archive(${ARCHIVE})

TEST_FEATURE("hdf5" WITH_HDF5)

if (WITH_HDF5)
    set(HDF5_PATCH ${CMAKE_CURRENT_LIST_DIR}/transitive-hdf5.patch)
endif ()

vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}
    PATCHES
        ${CMAKE_CURRENT_LIST_DIR}/no-install-deps.patch
        ${CMAKE_CURRENT_LIST_DIR}/config-pkg-location.patch
        ${CMAKE_CURRENT_LIST_DIR}/remove-libm-check.patch
        ${HDF5_PATCH}
)

if (VCPKG_CMAKE_SYSTEM_NAME STREQUAL Windows AND NOT CMAKE_HOST_WIN32)
    set (INIT_CACHE_ARG ${CMAKE_CURRENT_LIST_DIR}/TryRunResults-mingw.cmake)
endif ()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA # Disable this option if project cannot be built with Ninja
    INIT_CACHE ${INIT_CACHE_ARG}
    OPTIONS
        -DBUILD_UTILITIES=OFF
        -DBUILD_TESTING=OFF
        -DENABLE_EXAMPLES=OFF
        -DENABLE_TESTS=OFF
        -DENABLE_DYNAMIC_LOADING=OFF
        -DUSE_HDF5=${WITH_HDF5}
        -DENABLE_NETCDF_4=${WITH_HDF5}
        -DENABLE_DAP=OFF
        -DENABLE_DAP_REMOTE_TESTS=OFF
        -DDISABLE_INSTALL_DEPENDENCIES=ON
        -DConfigPackageLocation=share/netcdf
)

vcpkg_install_cmake()
vcpkg_fixup_cmake_targets(CONFIG_PATH share/netcdf)

if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/bin ${CURRENT_PACKAGES_DIR}/bin)
endif()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)

vcpkg_fixup_pkgconfig_file(NAMES netcdf)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/COPYRIGHT DESTINATION ${CURRENT_PACKAGES_DIR}/share/netcdf-c RENAME copyright)
