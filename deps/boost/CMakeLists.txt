SET(MAJOR 1)
SET(MINOR 62)
SET(REVISION 0)
SET(VERSION ${MAJOR}.${MINOR}.${REVISION})
SET(VERSION_UNDERSCORE ${MAJOR}_${MINOR}_${REVISION})
SET(NAME boost)

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(VARIANT debug)
ELSE ()
    SET(VARIANT release)
ENDIF()

IF (WIN32)
    SET(USER_CONFIG user-config-msvc.jam)
    SET(TOOLSET msvc-14.0)
    SET(BUILD_CMD b2)
    SET(TARBALL_EXTENSION 7z)
    SET(CONFIGURE_CMD bootstrap.bat)
    SET(PLATFORM_OPTIONS --layout=tagged)
ELSE ()
    SET(BUILD_CMD ./b2)
    IF (APPLE)
        SET(USER_CONFIG user-config-clang.jam)
        SET(TOOLSET clang)
    ELSE ()
        SET(USER_CONFIG user-config-gcc.jam)
        SET(TOOLSET gcc)
    ENDIF ()

    SET(CONFIGURE_CMD "./bootstrap.sh")
    SET(TARBALL_EXTENSION tar.bz2)
ENDIF ()

IF(HOST MATCHES ".*-musl")
    SET(TOOLSET gcc-musl)
    SET(USER_CONFIG user-config-musl.jam)
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    DEPENDS zlib
    URL http://downloads.sourceforge.net/project/${NAME}/${NAME}/${VERSION}/${NAME}_${VERSION_UNDERSCORE}.${TARBALL_EXTENSION} TIMEOUT 6000
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${CONFIGURE_CMD}
    BUILD_COMMAND ${BUILD_CMD}
        --user-config=${CMAKE_CURRENT_SOURCE_DIR}/${USER_CONFIG}
        address-model=64
        architecture=x86
        variant=${VARIANT}
        toolset=${TOOLSET}
        link=static
        threading=multi
        runtime-link=shared
        --with-iostreams
        --with-system
        --with-filesystem
        --with-program_options
        -s NO_BZIP2=1
        -s NO_ZLIB=0
        ${PLATFORM_OPTIONS}
        --prefix=${CMAKE_INSTALL_PREFIX} install
    INSTALL_COMMAND ""
)
