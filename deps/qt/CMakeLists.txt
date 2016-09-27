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
ELSE ()
    SET(PLATFORM linux-g++)
    SET(BUILD_CMD make)
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    URL http://download.qt.io/official_releases/${NAME}/${MAJOR}.${MINOR}/${VERSION}/single/qt-everywhere-opensource-src-${VERSION}.zip TIMEOUT 6000
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND call configure
        ${CONFIGURATIONS}
        -platform ${PLATFORM}
        -prefix ${CMAKE_INSTALL_PREFIX}
        -static -mp -opengl desktop -opensource -confirm-license -nomake tools -nomake examples -nomake tests
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
