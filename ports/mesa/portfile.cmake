include(vcpkg_common_functions)
set(VERSION_MAJOR 18)
set(VERSION_MINOR 2)
set(VERSION_REVISION 3)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION})
set(PACKAGE_NAME ${PORT}-${VERSION})
set(PACKAGE ${PACKAGE_NAME}.tar.xz)

vcpkg_from_gitlab(
    GITLAB_URL  https://gitlab.freedesktop.org
    OUT_SOURCE_PATH SOURCE_PATH
    REPO mesa/mesa
    REF ${PACKAGE_NAME}
    SHA512 10bb3f8e8236a454c3258cc3f7f30824405898539afa8e92165ad4a289cf2f3af1b62d32b5a79c32d6629fc720fa178dea408f32d7f8ee5e9d57e88ea1a724be
    HEAD_REF master
    PATCHES
        ${CMAKE_CURRENT_LIST_DIR}/swr-rast-ignore-CreateElementUnorderedAtomicMemCpy.patch
        ${CMAKE_CURRENT_LIST_DIR}/driver-error.patch
        ${CMAKE_CURRENT_LIST_DIR}/llvm-static.patch
)

set(PLATFORMS surfaceless)

TEST_FEATURE(drm PLATFORM_DRM)
TEST_FEATURE(x11 PLATFORM_X11)

if (PLATFORM_DRM)
    list(APPEND PLATFORMS drm)
endif()

if (PLATFORM_X11)
    list(APPEND PLATFORMS x11)
endif()

string(JOIN "," ENABLED_PLATFORMS ${PLATFORMS})

if (UNIX)
    #vcpkg_find_acquire_program(PYTHON2)
    set(VCPKG_BUILD_TYPE release)
    set(VCPKG_LIBRARY_LINKAGE dynamic)

    vcpkg_configure_meson(
        SOURCE_PATH ${SOURCE_PATH}
        OPTIONS
            -Dplatforms=${ENABLED_PLATFORMS}
            -Ddri2=false
            -Ddri3=false
            -Ddri-drivers=[]
            -Dgallium-drivers=swrast,swr
            -Dgallium-vdpau=false
            -Dgallium-xvmc=false
            -Dgallium-omx=disabled
            -Dgallium-va=false
            -Dopengl=true
            -Dglx=disabled
            -Degl=false
            -Dgles1=false
            -Dgles2=false
            -Dllvm=true
            -Dconfig-tool=${CURRENT_INSTALLED_DIR}/tools/llvm-config
            -Dshared-llvm=false
            -Dvalgrind=false
            -Dosmesa=gallium
            -Dvulkan-drivers=[]
    )

    vcpkg_install_meson()
else ()
    message(FATAL_ERROR "${PORT} is only supported on unix")
endif()

# Handle copyright
file(INSTALL ${CMAKE_CURRENT_LIST_DIR}/License.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
