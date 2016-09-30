SET(MAJOR 5)
SET(MINOR 7)
SET(REVISION 0)
SET(VERSION ${MAJOR}.${MINOR}.${REVISION})
SET(NAME qt)

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CONFIGURATIONS -debug)
ELSE ()
    SET(CONFIGURATIONS -release)
ENDIF()

IF (WIN32)
    SET(PLATFORM win32-msvc2015)
    SET(BUILD_CMD nmake)
    SET(CONFIGURE_CMD call configure)
    SET(TARBALL_EXTENSION zip)
    SET(PLATFORM_OPTIONS -mp -opengl desktop)
ELSE ()
    IF (APPLE)
        SET(PLATFORM macx-clang)
        SET(PLATFORM_OPTIONS -opengl desktop)
    ELSE ()
        SET(PLATFORM linux-g++)
        SET(PLATFORM_OPTIONS -qt-xcb)
    ENDIF ()

    SET(BUILD_CMD make)
    SET(CONFIGURE_CMD "./configure")
    SET(TARBALL_EXTENSION tar.xz)
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    URL http://download.qt.io/official_releases/${NAME}/${MAJOR}.${MINOR}/${VERSION}/single/qt-everywhere-opensource-src-${VERSION}.${TARBALL_EXTENSION} TIMEOUT 6000
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND ${CONFIGURE_CMD}
        ${CONFIGURATIONS}
        -platform ${PLATFORM}
        -prefix ${CMAKE_INSTALL_PREFIX}
        ${PLATFORM_OPTIONS}
        -opensource -confirm-license -nomake tools -nomake examples -nomake tests
        -no-dbus -no-audio-backend -no-qml-debug
        -skip qtconnectivity
        -skip qtdeclarative
        -skip qtgamepad
        -skip qtmultimedia
        -skip qtpurchasing
        -skip qtquickcontrols
        -skip qtquickcontrols2
        -skip qtscript
        -skip qtsensors
        -skip qtserialbus
        -skip qtserialport
        -skip qtscxml
        -skip qttools
        -skip qtwebchannel
        -skip qtwebengine
        -skip qtwebsockets
        -skip qtwebview
        -skip qtwinextras
        -skip qtxmlpatterns
    BUILD_COMMAND ${BUILD_CMD}
    INSTALL_COMMAND ${BUILD_CMD} install
)
