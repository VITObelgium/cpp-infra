cmake_policy(SET CMP0020 NEW)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Gui CONFIG REQUIRED)
find_package(Qt5Charts CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5QuickWidgets CONFIG REQUIRED)
find_package(Qt5QuickControls2 CONFIG REQUIRED)
find_package(Qt5Positioning CONFIG REQUIRED)
find_package(Qt5QuickCompiler REQUIRED)

option(STATIC_QT "Statically link qt" OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qtquick_compiler_add_resources(UI_RESOURCES resources.qrc)

function(find_qtdep)
    set (oneValueArgs NAME)
    set (multiValueArgs LIBNAME)
    cmake_parse_arguments(find_qtdep "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message(STATUS "Find library ${find_qtdep_NAME}")

    find_library(${find_qtdep_NAME}_LIBRARY NAMES ${find_qtdep_LIBNAME})
    find_library(${find_qtdep_NAME}_LIBRARY_DEBUG NAMES ${find_qtdep_LIBNAME}d)

    if (${find_qtdep_NAME}_LIBRARY AND ${find_qtdep_NAME}_LIBRARY_DEBUG)
        set(${find_qtdep_NAME}_LIBRARY optimized ${${find_qtdep_NAME}_LIBRARY} debug ${${find_qtdep_NAME}_LIBRARY_DEBUG} PARENT_SCOPE)
        message(STATUS "${find_qtdep_NAME} library: ${${find_qtdep_NAME}_LIBRARY}")
    endif ()

    add_library(${find_qtdep_NAME} STATIC IMPORTED)

    if (${find_qtdep_NAME}_LIBRARY)
        set_property(TARGET ${find_qtdep_NAME} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(${find_qtdep_NAME} PROPERTIES IMPORTED_LOCATION_RELEASE "${${find_qtdep_NAME}_LIBRARY}")
    endif()

    if (${find_qtdep_NAME}_LIBRARY_DEBUG)
        set_property(TARGET ${find_qtdep_NAME} APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(${find_qtdep_NAME} PROPERTIES IMPORTED_LOCATION_DEBUG "${${find_qtdep_NAME}_LIBRARY_DEBUG}")
    endif()
endfunction()

function(find_qtplugin)
    set (oneValueArgs NAME LIBNAME SUBDIR)
    cmake_parse_arguments(find_qtplugin "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message(STATUS "Find plugin ${find_qtplugin_NAME}")

    find_library(${find_qtplugin_NAME}_LIBRARY NAMES ${find_qtplugin_LIBNAME} HINTS ${CMAKE_PREFIX_PATH}/${find_qtplugin_SUBDIR})
    find_library(${find_qtplugin_NAME}_LIBRARY_DEBUG NAMES ${find_qtplugin_LIBNAME}d HINTS ${CMAKE_PREFIX_PATH}/debug/${find_qtplugin_SUBDIR} ${CMAKE_PREFIX_PATH}/${find_qtplugin_SUBDIR})

    if (${find_qtplugin_NAME}_LIBRARY AND ${find_qtplugin_NAME}_LIBRARY_DEBUG)
        set(${find_qtplugin_NAME}_LIBRARY optimized ${${find_qtplugin_NAME}_LIBRARY} debug ${${find_qtplugin_NAME}_LIBRARY_DEBUG} PARENT_SCOPE)
        message(STATUS "${find_qtplugin_NAME} library: ${${find_qtplugin_NAME}_LIBRARY}")
    endif ()

    if (NOT ${find_qtplugin_NAME}_LIBRARY)
        message(FATAL_ERROR "${find_qtplugin_NAME} library not found")
    endif ()

    add_library(${find_qtplugin_NAME} STATIC IMPORTED)
    if (${find_qtplugin_NAME}_LIBRARY)
        set_property(TARGET ${find_qtplugin_NAME} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(${find_qtplugin_NAME} PROPERTIES IMPORTED_LOCATION_RELEASE "${${find_qtplugin_NAME}_LIBRARY}")
    endif()

    if (${find_qtplugin_NAME}_LIBRARY_DEBUG)
        set_property(TARGET ${find_qtplugin_NAME} APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(${find_qtplugin_NAME} PROPERTIES IMPORTED_LOCATION_DEBUG "${${find_qtplugin_NAME}_LIBRARY_DEBUG}")
    endif()
endfunction()

set(QT_UI_FILES
    runsimulationdialog.ui
    mappingview.ui
    mainwindow.ui
    mapview.ui
    opaqview.ui
    opaqviewer.ui
    opaqvalidation.ui
)
qt5_wrap_ui(OPAQ_FORMS_HEADERS ${QT_UI_FILES})

add_executable(opaqui
    mapview.qml
    main.cpp
    imageprovider.h imageprovider.cpp
    jobrunner.h jobrunner.cpp
    mainwindow.h mainwindow.cpp
    runsimulationdialog.h runsimulationdialog.cpp
    stationresultsmodel.h stationresultsmodel.cpp
    validationresultsmodel.h validationresultsmodel.cpp
    mappingview.h mappingview.cpp
    mapview.h mapview.cpp
    opaqview.h opaqview.cpp
    opaqviewer.h opaqviewer.cpp
    opaqvalidation.h opaqvalidation.cpp
    resultsview.h resultsview.cpp
    typeregistrations.h
    validationlineview.h validationlineview.cpp
    validationscatterview.h validationscatterview.cpp
    ${UI_RESOURCES}
    ${OPAQ_FORMS_HEADERS}
)

source_group("Qt ui files" FILES ${QT_UI_FILES})
source_group("Qt generated" FILES ${UI_RESOURCES} ${OPAQ_FORMS_HEADERS})
set_target_properties(opaqui PROPERTIES AUTOMOC ON)

if (MSVC)
    set_target_properties(opaqui PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
elseif (MINGW)
    set_target_properties(opaqui PROPERTIES LINK_FLAGS_RELEASE "-Wl,-subsystem,windows")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(opaqui
    opaqcore
    opaqplugins
    rio
    gdxcore
    uiinfra
    Qt5::Gui
    Qt5::Charts
    Qt5::Widgets
    Qt5::Core
    Qt5::QuickControls2
    Qt5::QuickWidgets
    Qt5::Positioning
)

install(TARGETS opaqui
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if (STATIC_QT)
    target_compile_definitions(opaqui PRIVATE "-DQT_CHARTS_STATICLIB")

    find_package(OpenGL)
    find_package(Freetype REQUIRED)
    find_package(Qt5Location REQUIRED)

    find_qtdep(NAME Qt5PositioningQuick LIBNAME Qt5PositioningQuick)
    
    find_qtplugin(NAME QtQuickLayoutsPlugin LIBNAME qquicklayoutsplugin SUBDIR qml/QtQuick/Layouts)
    find_qtplugin(NAME QtQuick2Plugin LIBNAME qtquick2plugin SUBDIR qml/QtQuick.2)
    find_qtplugin(NAME QtQuick2TemplatesPlugin LIBNAME qtquicktemplates2plugin SUBDIR qml/QtQuick/Templates.2)
    find_qtplugin(NAME QtQuick2WindowPlugin LIBNAME windowplugin SUBDIR qml/QtQuick/Window.2)
    find_qtplugin(NAME QtQuickControls2Plugin LIBNAME qtquickcontrols2plugin SUBDIR qml/QtQuick/Controls.2)
    find_qtplugin(NAME QtQuickControls2FusionPlugin LIBNAME qtquickcontrols2fusionstyleplugin SUBDIR qml/QtQuick/Controls.2/Fusion)
    find_qtplugin(NAME QtQuickLocationPlugin LIBNAME declarative_location SUBDIR qml/QtLocation)
    find_qtplugin(NAME QtQuickPositioningPlugin LIBNAME declarative_positioning SUBDIR qml/QtPositioning)

    find_qtplugin(NAME QGeoServicesEsriPlugin LIBNAME qtgeoservices_esri SUBDIR plugins/geoservices)
    find_qtplugin(NAME QGeoServicesOsmPlugin LIBNAME qtgeoservices_osm SUBDIR plugins/geoservices)
    find_qtplugin(NAME QGeoServicesItemsOverlayPlugin LIBNAME qtgeoservices_itemsoverlay SUBDIR plugins/geoservices)

    target_link_libraries(opaqui
        QtQuickLayoutsPlugin
        QtQuick2Plugin
        QtQuick2WindowPlugin
        QtQuick2TemplatesPlugin
        QtQuickControls2Plugin
        QtQuickControls2FusionPlugin
        QtQuickLocationPlugin
        QtQuickPositioningPlugin
        QGeoServicesEsriPlugin
        QGeoServicesOsmPlugin
        QGeoServicesItemsOverlayPlugin
        Qt5::QJpegPlugin
        Freetype::Freetype
    )

    if (MSVC)
        find_qtdep(NAME QtFontDatabaseSupport LIBNAME Qt5FontDatabaseSupport)
        find_qtdep(NAME QtThemeSupport LIBNAME Qt5ThemeSupport)
        find_qtdep(NAME QtEventDispatcherSupport LIBNAME Qt5EventDispatcherSupport)
        find_qtdep(NAME QtAccessibilitySupport LIBNAME Qt5AccessibilitySupport)
        find_qtdep(NAME QtUiAutomation LIBNAME Qt5WindowsUIAutomationSupport)

        target_link_libraries(opaqui
            Dwmapi.lib
            Wtsapi32.lib
            UxTheme.lib
            QtThemeSupport
            QtFontDatabaseSupport
            QtEventDispatcherSupport
            QtAccessibilitySupport
            QtUiAutomation
            Qt5::QWindowsVistaStylePlugin
            Qt5::QWindowsIntegrationPlugin
        )
    endif ()
endif ()
