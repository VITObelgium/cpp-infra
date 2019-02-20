include(vcpkg_common_functions)
set(VERSION_MAJOR 4)
set(VERSION_MINOR 6)
set(VERSION_REVISION 2)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION})

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Unidata/netcdf-c
    REF v${VERSION}
    SHA512 7c7084e80cf2fb86cd05101f5be7b74797ee96bf49afadfae6ab32ceed6cd9a049bfa90175e7cc0742806bcd2f61156e33fe7930c7b646661d9c89be6b20dea3
    HEAD_REF master
    PATCHES
        no-install-deps.patch
        config-pkg-location.patch
        remove-libm-check.patch
        nc-config-zlib.patch
        mingw.patch
)

file(REMOVE ${SOURCE_PATH}/cmake/modules/FindZLIB.cmake)
file(REMOVE ${SOURCE_PATH}/cmake/modules/windows/FindHDF5.cmake)

TEST_FEATURE("hdf5" WITH_HDF5)
if (WITH_HDF5)
    vcpkg_apply_patches(
        SOURCE_PATH ${SOURCE_PATH}
        PATCHES
            transitive-hdf5.patch
            hdf5-targets.patch
    )
endif ()

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
    OPTIONS_RELEASE
        ${HDF5_OPTIONS_REL}
    OPTIONS_DEBUG
        ${HDF5_OPTIONS_DBG}
)

vcpkg_install_cmake()
vcpkg_fixup_cmake_targets(CONFIG_PATH share/netcdf)

vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/bin/nc-config "${CURRENT_PACKAGES_DIR}" "${CURRENT_INSTALLED_DIR}")
if(VCPKG_LIBRARY_LINKAGE STREQUAL "static" AND WITH_HDF5)
    vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/bin/nc-config "-lnetcdf" "-lnetcdf -lhdf5_hl -lhdf5")
endif()

file(INSTALL ${CURRENT_PACKAGES_DIR}/bin/nc-config DESTINATION ${CURRENT_PACKAGES_DIR}/tools
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/bin ${CURRENT_PACKAGES_DIR}/bin)
endif()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)

vcpkg_fixup_pkgconfig_file(NAMES netcdf)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/COPYRIGHT DESTINATION ${CURRENT_PACKAGES_DIR}/share/netcdf-c RENAME copyright)
