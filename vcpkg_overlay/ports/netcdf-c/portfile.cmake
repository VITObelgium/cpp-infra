vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Unidata/netcdf-c
    REF v4.9.2
    SHA512 e0c299843083cde54bfaccebd4f831513c2c531f3a98e37a1bc14d12a5e63af0b994cab9292bcb17e1b162ffe26b49ed3f9c6de7f2f48cdfcfd3f3c4a377bb04
    HEAD_REF master
    PATCHES
    no-install-deps.patch
    fix-dependency-zlib.patch
    use_targets.patch
    cmakeconfig.patch
    fix-linkage-error.patch
    fix-pkgconfig.patch
    fix-manpage-msys.patch
    fix-dependency-mpi.patch
)

# Remove outdated find modules
file(REMOVE "${SOURCE_PATH}/cmake/modules/FindSZIP.cmake")
file(REMOVE "${SOURCE_PATH}/cmake/modules/FindZLIB.cmake")
file(REMOVE "${SOURCE_PATH}/cmake/modules/windows/FindHDF5.cmake")

if(NOT VCPKG_TARGET_IS_WINDOWS OR VCPKG_TARGET_IS_MINGW)
    set(CRT_OPTION "")
elseif(VCPKG_CRT_LINKAGE STREQUAL "static")
    set(CRT_OPTION -DNC_USE_STATIC_CRT=ON)
else()
    set(CRT_OPTION -DNC_USE_STATIC_CRT=OFF)
endif()

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
    dap ENABLE_DAP
    netcdf-4 ENABLE_NETCDF_4
    hdf5 ENABLE_HDF5
    nczarr ENABLE_NCZARR
    nczarr-zip ENABLE_NCZARR_ZIP
    tools BUILD_UTILITIES
    parallel ENABLE_PARALLEL4
)

if(NOT ENABLE_DAP AND NOT ENABLE_NCZARR)
    list(APPEND FEATURE_OPTIONS "-DCMAKE_DISABLE_FIND_PACKAGE_CURL=ON;-DENABLE_BYTERANGE=OFF")
endif()

if(ENABLE_HDF5)
    # Fix hdf5 szip support detection for static linkage.
    x_vcpkg_pkgconfig_get_modules(
        PREFIX HDF5
        MODULES hdf5
        LIBRARIES
    )

    if(HDF5_LIBRARIES_RELEASE MATCHES "szip")
        list(APPEND FEATURE_OPTIONS "-DUSE_SZIP=ON")
    endif()
endif()

if(VCPKG_TARGET_IS_UWP)
    string(APPEND VCPKG_C_FLAGS " /wd4996 /wd4703")
    string(APPEND VCPKG_CXX_FLAGS " /wd4996 /wd4703")
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    DISABLE_PARALLEL_CONFIGURE # netcdf-c configures in the source!
    OPTIONS
    -DBUILD_TESTING=OFF
    -DENABLE_EXAMPLES=OFF
    -DENABLE_TESTS=OFF
    -DENABLE_PLUGINS=OFF
    -DENABLE_FILTER_TESTING=OFF
    -DENABLE_HDF4_FILE_TESTS=OFF
    -DENABLE_DAP_REMOTE_TESTS=OFF
    -DENABLE_LIBXML2=OFF
    -DDISABLE_INSTALL_DEPENDENCIES=ON
    -DNETCDF_ENABLE_FILTER_ZSTD=OFF
    ${CRT_OPTION}
    ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "netcdf" CONFIG_PATH "lib/cmake/netCDF")
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/bin/nc-config" "${CURRENT_PACKAGES_DIR}/bin/nc-config") # invalid

if("tools" IN_LIST FEATURES)
    vcpkg_copy_tools(
        TOOL_NAMES nccopy ncdump ncgen ncgen3
        AUTO_CLEAN
    )
elseif(VCPKG_LIBRARY_LINKAGE STREQUAL "static" OR NOT VCPKG_TARGET_IS_WINDOWS)
    # delete bin under non-windows because the dynamic libraries get put in lib
    file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/bin" "${CURRENT_PACKAGES_DIR}/bin")
endif()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

file(INSTALL "${SOURCE_PATH}/COPYRIGHT" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
