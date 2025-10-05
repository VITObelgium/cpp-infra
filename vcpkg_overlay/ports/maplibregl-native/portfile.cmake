if(VCPKG_HOST_IS_WIN32)
    vcpkg_add_to_path($ENV{ProgramFiles}/git/bin)
    vcpkg_add_to_path($ENV{ProgramW6432}/git/bin)
endif()

vcpkg_find_acquire_program(GIT)

file(REMOVE_RECURSE ${CURRENT_BUILDTREES_DIR}/src)

set(GIT_URL "https://github.com/maplibre/maplibre-native.git")
set(GIT_REV "android-v10.1.0")
set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/${PORT})

if(EXISTS ${SOURCE_PATH})
    message(STATUS "Removing existing source directory")
    file(REMOVE_RECURSE ${SOURCE_PATH})
endif()

make_directory(${SOURCE_PATH})

message(STATUS "Cloning and fetching submodules")
vcpkg_execute_required_process(
    COMMAND ${GIT} clone --config core.longpaths=true --recurse-submodules ${GIT_URL} ${SOURCE_PATH}
    WORKING_DIRECTORY ${SOURCE_PATH}
    LOGNAME clone
)

message(STATUS "Checkout revision ${GIT_REV}")
vcpkg_execute_required_process(
    COMMAND ${GIT} checkout ${GIT_REV}
    WORKING_DIRECTORY ${SOURCE_PATH}
    LOGNAME checkout
)

vcpkg_execute_required_process(
    COMMAND ${GIT} submodule update --init
    WORKING_DIRECTORY ${SOURCE_PATH}
    LOGNAME update-submodules-${TARGET_TRIPLET}
)

vcpkg_execute_required_process(
    COMMAND ${GIT} submodule update --init
    WORKING_DIRECTORY ${SOURCE_PATH}/vendor/mapbox-base
    LOGNAME update-mapbox-submodules-${TARGET_TRIPLET}
)

vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}
    PATCHES
    cmake-changes.patch
    timer-overflow.patch
    boost-numeric.patch
    fix-includes.patch
    http2.patch
)

if(VCPKG_TARGET_IS_OSX)
    set(ADDITIONAL_ARGS -DCMAKE_CXX_FLAGS=-D_HAS_AUTO_PTR_ETC=0)
endif()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
    -DMLN_WITH_RTTI=ON
    -DMLN_WITH_COVERAGE=OFF
    -DMLN_WITH_WERROR=OFF
    -DMLN_WITH_QT=ON
    -DMLN_QT_LIBRARY_ONLY=ON
    -DMLN_QT_STATIC=ON
    -DMLN_QT_WITH_INTERNAL_SQLITE=ON
    ${ADDITIONAL_ARGS}
)

vcpkg_install_cmake()
vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/QMapLibreGL TARGET_PATH share/QMapLibreGL)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

file(INSTALL ${SOURCE_PATH}/LICENSE.md DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
