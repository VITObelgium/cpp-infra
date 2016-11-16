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

IF (MSVC)
    SET(PLATFORM win32-msvc2015)
    SET(BUILD_CMD nmake)
    SET(CONFIGURE_CMD call configure)
    SET(TARBALL_EXTENSION zip)
    SET(PLATFORM_OPTIONS -mp)
ELSE ()
    IF (APPLE)
        SET(PLATFORM macx-clang)
        SET(PLATFORM_OPTIONS -opengl desktop)
    ELSEIF (MINGW)
        SET(PLATFORM win32-g++)
        SET(PLATFORM_OPTIONS -static -opengl desktop -no-opengl)
        IF (CMAKE_CROSSCOMPILING)
            SET(PLATFORM_OPTIONS ${PLATFORM_OPTIONS} -device-option CROSS_COMPILE=x86_64-w64-mingw32-)
        ENDIF ()
    ELSE ()
        SET(PLATFORM linux-g++)
        SET(PLATFORM_OPTIONS -qt-xcb)
    ENDIF ()

    SET(BUILD_CMD make -j3)
    SET(CONFIGURE_CMD "./configure")
    SET(TARBALL_EXTENSION tar.xz)
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    DEPENDS zlib sqlite
    URL http://download.qt.io/official_releases/${NAME}/${MAJOR}.${MINOR}/${VERSION}/single/qt-everywhere-opensource-src-${VERSION}.${TARBALL_EXTENSION} TIMEOUT 6000
    BUILD_IN_SOURCE 1
    PATCH_COMMAND ${CMAKE_COMMAND} -DQT_SRC_ROOT=${CMAKE_CURRENT_BINARY_DIR}/qt-prefix/src -P ${CMAKE_CURRENT_SOURCE_DIR}/static_runtime.cmake
    CONFIGURE_COMMAND ${CONFIGURE_CMD}
        -I${CMAKE_INSTALL_PREFIX}/include
        -L${CMAKE_INSTALL_PREFIX}/lib
        ${CONFIGURATIONS}
        -platform ${PLATFORM}
        -prefix ${CMAKE_INSTALL_PREFIX}
        ${PLATFORM_OPTIONS}
        -opensource -confirm-license -nomake tools -nomake examples -nomake tests
        -no-openssl -no-dbus -no-audio-backend -no-qml-debug
        -qt-sql-sqlite -system-sqlite
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
