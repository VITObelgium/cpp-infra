
add_library(Qt5::QWindowsIntegrationPlugin MODULE IMPORTED)

_populate_Gui_plugin_properties(QWindowsIntegrationPlugin RELEASE "platforms/qwindows.lib")

if (EXISTS "${_qt5Gui_install_prefix}/plugins/platforms/qwindowsd.lib" )
    _populate_Gui_plugin_properties(QWindowsIntegrationPlugin DEBUG "platforms/qwindowsd.lib")
endif()

list(APPEND Qt5Gui_PLUGINS Qt5::QWindowsIntegrationPlugin)
