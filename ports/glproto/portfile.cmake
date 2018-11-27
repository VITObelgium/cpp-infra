include(vcpkg_common_functions)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 4)
set(VERSION_REVISION 17)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION})

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO freedesktop/glproto
    REF ${PORT}-${VERSION}
    SHA512 16f0347952bf5892f04ce9664bd3d35a05a1f292700eacdfe7a3423e08b60c6bb1660156b94245e4c71f7ec88fa1c3545fb37322a83863d0521e05c4d5262df8
)

if (UNIX AND NOT APPLE)
    set(VCPKG_BUILD_TYPE release)

    vcpkg_execute_required_process(
        COMMAND autoreconf -v --install -I ${CURRENT_INSTALLED_DIR}/lib/aclocal
        WORKING_DIRECTORY ${SOURCE_PATH}
        LOGNAME autogen-${TARGET_TRIPLET}-rel
    )

    vcpkg_configure_autoconf(
        SOURCE_PATH ${SOURCE_PATH}
        RUN_AUTOGEN
        OPTIONS
    )

    vcpkg_build_autotools()
    vcpkg_install_autotools()
else ()
    message(FATAL_ERROR "glproto is only supported on unix")
endif()

# Handle copyright
file(INSTALL ${SOURCE_PATH}/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
