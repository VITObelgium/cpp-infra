cmake_policy(SET CMP0020 NEW)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Gui)
find_package(Qt5Charts)
find_package(Qt5Widgets)

option(STATIC_QT "Statically link qt" OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt5_wrap_ui(OPAQ_FORMS_HEADERS runsimulationdialog.ui opaqview.ui opaqviewer.ui opaqvalidation.ui)

add_executable(opaqui
    main.cpp
    uiutils.h
    mainwindow.h mainwindow.cpp
    runsimulationdialog.h runsimulationdialog.cpp
    stationresultsmodel.h stationresultsmodel.cpp
    validationresultsmodel.h validationresultsmodel.cpp
    opaqview.h opaqview.cpp
    opaqviewer.h opaqviewer.cpp
    opaqvalidation.h opaqvalidation.cpp
    resultsview.h resultsview.cpp
    validationlineview.h validationlineview.cpp
    validationscatterview.h validationscatterview.cpp
    ${OPAQ_FORMS_HEADERS}
)

source_group(ui FILES ${OPAQ_FORMS_HEADERS} opaqui_automoc.cpp)
set_target_properties(opaqui PROPERTIES AUTOMOC ON)

if (MSVC)
    set_target_properties(opaqui PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
elseif (MINGW)
    set_target_properties(opaqui PROPERTIES LINK_FLAGS "-Wl,-subsystem,windows")
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(opaqui opaqcore opaqplugins Qt5::Gui Qt5::Charts Qt5::Widgets Qt5::Core ${EXTRA_LIBS})

install(TARGETS opaqui
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if (STATIC_QT)
    add_definitions("-DQT_CHARTS_STATICLIB")

    find_package(OpenGL)
    #find_dependency(NAME QT_PLATFORMSUPPORT LIBNAMES Qt5PlatformSupport DEBUGLIBNAMES Qt5PlatformSupportd)
    find_dependency(NAME QT_HARFBUZZ LIBNAMES qtharfbuzz DEBUGLIBNAMES qtharfbuzzd)
    find_dependency(NAME QT_PCRE LIBNAMES qtpcre2 DEBUGLIBNAMES qtpcre2d)

    if (MSVC)
        find_dependency(NAME QT_FREETYPE LIBNAMES qtfreetype DEBUGLIBNAMES qtfreetyped)
    else ()
        find_package(Png REQUIRED)
        list (APPEND Qt5Gui_LIBRARIES ${Png_LIBRARY})
    endif ()

    if (WIN32)
        # Hack to force opengl lib behind Qt5Gui lib
        list (APPEND Qt5Gui_LIBRARIES ${OPENGL_LIBRARIES})
        if (MSVC)
            list (APPEND Qt5Gui_LIBRARIES ${QT_FREETYPE_LIBRARY})
        endif ()

        target_link_libraries(opaqui
            Qt5::QWindowsIntegrationPlugin
            ${QT_PCRE_LIBRARY}
            ${QT_HARFBUZZ_LIBRARY}
            ${QT_PLATFORMSUPPORT_LIBRARY}
            imm32 winmm ws2_32
            ${Qt5Gui_LIBRARIES}
        )
    elseif (APPLE)
        target_link_libraries(opaqui Qt5::QCocoaIntegrationPlugin ${OPENGL_LIBRARIES})
    endif ()
endif ()
