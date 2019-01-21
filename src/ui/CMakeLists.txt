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

set(QT_UI_FILES
    fileselectioncombobox.ui
    mappingview.ui
    mainwindow.ui
    mapview.ui
    opaqview.ui
    opaqviewer.ui
    opaqvalidation.ui
    preferencesdialog.ui
    runsimulationdialog.ui
)
qt5_wrap_ui(OPAQ_FORMS_HEADERS ${QT_UI_FILES})

add_executable(opaqui
    mapview.qml
    main.cpp
    rasterdisplaydata.h
    fileselectioncombobox.h fileselectioncombobox.cpp
    jobrunner.h jobrunner.cpp
    legendsettings.h
    mainwindow.h mainwindow.cpp
    runsimulationdialog.h runsimulationdialog.cpp
    stationresultsmodel.h stationresultsmodel.cpp
    validationresultsmodel.h validationresultsmodel.cpp
    mappingview.h mappingview.cpp
    mapview.h mapview.cpp
    opaqview.h opaqview.cpp
    opaqviewer.h opaqviewer.cpp
    opaqvalidation.h opaqvalidation.cpp
    pointsourcemodel.h pointsourcemodel.cpp
    preferencesdialog.h preferencesdialog.cpp
    resultsview.h resultsview.cpp
    typeregistrations.h
    validationlineview.h validationlineview.cpp
    validationscatterview.h validationscatterview.cpp
    opaq.rc
    ${UI_RESOURCES}
    ${OPAQ_FORMS_HEADERS}
)

source_group("Qt ui files" FILES ${QT_UI_FILES})
source_group("Qt generated" FILES ${UI_RESOURCES} ${OPAQ_FORMS_HEADERS})
set_target_properties(opaqui PROPERTIES AUTOMOC ON)

if (MSVC)
    target_sources(opaqui PRIVATE opaq.rc ${CMAKE_BINARY_DIR}/version.rc)
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
    include(FindQtPlugin)

    target_compile_definitions(opaqui PRIVATE "-DQT_CHARTS_STATICLIB")

    find_package(Qt5Svg REQUIRED)
    find_package(Qt5Location REQUIRED)
    
    find_qtplugin(NAME QtQuickLayoutsPlugin LIBNAME qquicklayoutsplugin SUBDIR qml/QtQuick/Layouts)
    find_qtplugin(NAME QtQuick2Plugin LIBNAME qtquick2plugin SUBDIR qml/QtQuick.2)
    find_qtplugin(NAME QtQuick2TemplatesPlugin LIBNAME qtquicktemplates2plugin SUBDIR qml/QtQuick/Templates.2)
    find_qtplugin(NAME QtQuick2WindowPlugin LIBNAME windowplugin SUBDIR qml/QtQuick/Window.2)
    find_qtplugin(NAME QtQuickControls2Plugin LIBNAME qtquickcontrols2plugin SUBDIR qml/QtQuick/Controls.2)
    find_qtplugin(NAME QtQuickControls2FusionPlugin LIBNAME qtquickcontrols2fusionstyleplugin SUBDIR qml/QtQuick/Controls.2/Fusion)
    find_qtplugin(NAME QtQuickLocationPlugin LIBNAME declarative_location SUBDIR qml/QtLocation)
    find_qtplugin(NAME QtQuickPositioningPlugin LIBNAME declarative_positioning SUBDIR qml/QtPositioning)
    
    target_link_libraries(opaqui
        QtQuickLayoutsPlugin
        QtQuick2Plugin
        QtQuick2WindowPlugin
        QtQuick2TemplatesPlugin
        QtQuickControls2Plugin
        QtQuickControls2FusionPlugin
        QtQuickLocationPlugin
        QtQuickPositioningPlugin
        Qt5::Location
        Qt5::GeoServiceProviderFactoryEsri
        Qt5::QGeoServiceProviderFactoryOsm
        Qt5::QGeoServiceProviderFactoryItemsOverlay
        Qt5::QSvgPlugin
        Qt5::QJpegPlugin
    )

    if (MSVC)
        target_link_libraries(opaqui
            Qt5::QWindowsVistaStylePlugin
            Qt5::QWindowsIntegrationPlugin
        )
    elseif(APPLE)
        find_package(Qt5Sql REQUIRED)

        target_link_libraries(opaqui
            Qt5::Sql
            Qt5::QSQLiteDriverPlugin
            Qt5::QMacStylePlugin
            Qt5::QCocoaIntegrationPlugin
        )
    endif ()

    if (NOT MSVC)
        find_qtplugin(NAME QGeoServicesMapboxglPlugin LIBNAME qtgeoservices_mapboxgl SUBDIR plugins/geoservices)
        target_link_libraries(opaqui QMapboxgl QGeoServicesMapboxglPlugin)
    endif ()
endif ()
