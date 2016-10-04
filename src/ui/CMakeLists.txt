CMAKE_POLICY(SET CMP0020 NEW)
SET(CMAKE_AUTOMOC ON)
FIND_PACKAGE(Qt5Gui)
FIND_PACKAGE(Qt5Charts)
FIND_PACKAGE(Qt5Widgets)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
)

QT5_WRAP_UI(OPAQ_FORMS_HEADERS runsimulationdialog.ui opaqview.ui opaqviewer.ui)

ADD_EXECUTABLE(opaqui
    main.cpp
    uiutils.h
    mainwindow.h mainwindow.cpp
    runsimulationdialog.h runsimulationdialog.cpp
    stationresultsmodel.h stationresultsmodel.cpp
    predictionresultsmodel.h predictionresultsmodel.cpp
    opaqview.h opaqview.cpp
    opaqviewer.h opaqviewer.cpp
    resultsview.h resultsview.cpp
    ${OPAQ_FORMS_HEADERS}
)

SOURCE_GROUP(ui FILES ${OPAQ_FORMS_HEADERS} opaqui_automoc.cpp)

IF (MSVC)
    SET_TARGET_PROPERTIES(opaqui PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
ENDIF ()

TARGET_LINK_LIBRARIES(opaqui opaqcore Qt5::Gui Qt5::Charts Qt5::Widgets ${EXTRA_LIBS})

IF (STATIC_QT)
    FIND_PACKAGE(OpenGL)
    
    FIND_PACKAGE(Qt5PrintSupport)
    FIND_LIBRARY(QT_PLATFORMSUPPORT_LIBRARY NAMES Qt5PlatformSupport)
    FIND_LIBRARY(QT_PLATFORMSUPPORT_LIBRARY_DEBUG NAMES Qt5PlatformSupportd)

    FIND_LIBRARY(QT_HARFBUZZ_LIBRARY NAMES qtharfbuzzng)
    FIND_LIBRARY(QT_HARFBUZZ_LIBRARY_DEBUG NAMES qtharfbuzzngd)

    FIND_LIBRARY(QT_FREETYPE_LIBRARY NAMES qtfreetype)
    FIND_LIBRARY(QT_FREETYPE_LIBRARY_DEBUG NAMES qtfreetyped)

    FIND_LIBRARY(QT_PCRE_LIBRARY NAMES qtpcre)
    FIND_LIBRARY(QT_PCRE_LIBRARY_DEBUG NAMES qtpcred)

    IF (WIN32)
        TARGET_LINK_LIBRARIES(opaqui
            Qt5::QWindowsIntegrationPlugin ${OPENGL_LIBRARIES} imm32 winmm
            debug ${QT_HARFBUZZ_LIBRARY_DEBUG} optimized ${QT_HARFBUZZ_LIBRARY}
            debug ${QT_PLATFORMSUPPORT_LIBRARY_DEBUG} optimized ${QT_PLATFORMSUPPORT_LIBRARY}
            debug ${QT_FREETYPE_LIBRARY_DEBUG} optimized ${QT_FREETYPE_LIBRARY}
            debug ${QT_PCRE_LIBRARY_DEBUG} optimized ${QT_PCRE_LIBRARY})
    ELSEIF (APPLE)
        TARGET_LINK_LIBRARIES(opaqui Qt5::QCocoaIntegrationPlugin ${OPENGL_LIBRARIES})
    ENDIF ()
ENDIF ()
