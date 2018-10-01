include(vcpkg_common_functions)

set(MAJOR 3)
set(MINOR 5)
set(REVISION 2)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO python-cmake-buildsystem/python-cmake-buildsystem
    REF c3b8d532242efed85fb3cd77b0e39e424b6db475
    SHA512 90f09bc7cca5cc63979df885262c696506473ab38022ad21ae73d0556cf7b6956f47098f70e2c825fc057feb72365eda78af4bf13075677ec4ce3b1b372c5606
    HEAD_REF master
)

if (NOT VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message (FATAL_ERROR "The python build is only supported on linux")
endif ()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DCMAKE_POLICY_DEFAULT_CMP0063=OLD
        -DBUILD_EXTENSIONS_AS_BUILTIN=ON
        -DWITH_STATIC_DEPENDENCIES=ON
        -DINSTALL_MANUAL=OFF
        -DINSTALL_TEST=OFF
        -DENABLE_CTYPES_TEST=OFF
        -DENABLE_TESTCAPI=OFF
        -DUSE_SYSTEM_LIBRARIES=OFF
        -DUSE_SYSTEM_ZLIB=ON
        -DPYTHON_VERSION=${VERSION}
)

vcpkg_install_cmake()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/bin)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/lib/python3.5/lib-dynload)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/share)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/lib/pkgconfig)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/lib/python3.5/lib-dynload)

file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/tools)
file(RENAME ${CURRENT_PACKAGES_DIR}/bin/python ${CURRENT_PACKAGES_DIR}/tools/python3)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/bin)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/../../Python-${VERSION}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)

