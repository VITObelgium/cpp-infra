include(vcpkg_common_functions)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 19)
set(VERSION_REVISION 1)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION})

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO freedesktop/xorg-macros
    REF util-macros-${VERSION}
    SHA512 627215e3d3a8870aba5a80441c6fc4018b7cdadef918e6aa5bdc4a36e954b02daa9f1081147b6d420fc0220f327b381c109ac27b337a505238bfc8100d8f077b
)

if (UNIX)
    set(VCPKG_BUILD_TYPE release)
    set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

    vcpkg_execute_required_process(
        COMMAND autoreconf -v --install
        WORKING_DIRECTORY ${SOURCE_PATH}
        LOGNAME autogen-${TARGET_TRIPLET}-rel
    )

    vcpkg_configure_autoconf(
        SOURCE_PATH ${SOURCE_PATH}
        OPTIONS
            --datadir=${CURRENT_PACKAGES_DIR}/lib
    )

    vcpkg_build_autotools()
    vcpkg_install_autotools()
    vcpkg_fixup_pkgconfig_file()
else ()
    message(FATAL_ERROR "xorg-macros is only supported on unix")
endif()

# Handle copyright
file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
