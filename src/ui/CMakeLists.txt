cmake_policy(SET CMP0020 NEW)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Gui)
find_package(Qt5Charts)
find_package(Qt5Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt5_wrap_ui(OPAQ_FORMS_HEADERS runsimulationdialog.ui opaqview.ui opaqviewer.ui opaqvalidation.ui)

add_executable(opaqui
    main.cpp
    uiutils.h
    mainwindow.h mainwindow.cpp
    runsimulationdialog.h runsimulationdialog.cpp
    stationresultsmodel.h stationresultsmodel.cpp
    predictionresultsmodel.h predictionresultsmodel.cpp
    opaqview.h opaqview.cpp
    opaqviewer.h opaqviewer.cpp
    opaqvalidation.h opaqvalidation.cpp
    resultsview.h resultsview.cpp
    ${OPAQ_FORMS_HEADERS}
)

source_group(ui FILES ${OPAQ_FORMS_HEADERS} opaqui_automoc.cpp)

if (MSVC)
    set_target_properties(opaqui PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif ()

target_link_libraries(opaqui opaqcore Qt5::Gui Qt5::Charts Qt5::Widgets ${EXTRA_LIBS})

if (STATIC_QT)
    find_package(OpenGL)

    find_package(Qt5PrintSupport)
    find_library(QT_PLATFORMSUPPORT_LIBRARY NAMES Qt5PlatformSupport)
    find_library(QT_PLATFORMSUPPORT_LIBRARY_DEBUG NAMES Qt5PlatformSupportd)

    find_library(QT_HARFBUZZ_LIBRARY NAMES qtharfbuzzng)
    find_library(QT_HARFBUZZ_LIBRARY_DEBUG NAMES qtharfbuzzngd)

    find_library(QT_FREETYPE_LIBRARY NAMES qtfreetype)
    find_library(QT_FREETYPE_LIBRARY_DEBUG NAMES qtfreetyped)

    find_library(QT_PCRE_LIBRARY NAMES qtpcre)
    find_library(QT_PCRE_LIBRARY_DEBUG NAMES qtpcred)

    if (WIN32)
        target_link_libraries(opaqui
            Qt5::QWindowsIntegrationPlugin ${OPENGL_LIBRARIES} imm32 winmm
            debug ${QT_HARFBUZZ_LIBRARY_DEBUG} optimized ${QT_HARFBUZZ_LIBRARY}
            debug ${QT_PLATFORMSUPPORT_LIBRARY_DEBUG} optimized ${QT_PLATFORMSUPPORT_LIBRARY}
            debug ${QT_FREETYPE_LIBRARY_DEBUG} optimized ${QT_FREETYPE_LIBRARY}
            debug ${QT_PCRE_LIBRARY_DEBUG} optimized ${QT_PCRE_LIBRARY})
    elseif (APPLE)
        target_link_libraries(opaqui Qt5::QCocoaIntegrationPlugin ${OPENGL_LIBRARIES})
    endif ()
endif ()
