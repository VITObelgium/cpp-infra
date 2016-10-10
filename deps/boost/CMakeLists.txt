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
    SET(TOOLSET msvc-14.0)
    SET(BUILD_CMD b2)
    SET(TARBALL_EXTENSION 7z)
    SET(CONFIGURE_CMD bootstrap.bat)
    SET(PLATFORM_OPTIONS --layout=tagged)
ELSE ()
    SET(BUILD_CMD ./b2)
    IF (APPLE)
        SET(TOOLSET clang cxxflags=-std=c++14)
    ELSE ()
        SET(TOOLSET gcc cxxflags=-std=c++14)
    ENDIF ()

    SET(CONFIGURE_CMD "./bootstrap.sh")
    SET(TARBALL_EXTENSION tar.bz2)
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    URL http://downloads.sourceforge.net/project/${NAME}/${NAME}/${VERSION}/${NAME}_${VERSION_UNDERSCORE}.${TARBALL_EXTENSION} TIMEOUT 6000
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${CONFIGURE_CMD}
    BUILD_COMMAND ${BUILD_CMD}
        address-model=64
        variant=${VARIANT}
        toolset=${TOOLSET}
        link=static
        threading=multi
        runtime-link=shared
        --with-math
        ${PLATFORM_OPTIONS}
        --prefix=${CMAKE_INSTALL_PREFIX} install
    INSTALL_COMMAND ""
)
