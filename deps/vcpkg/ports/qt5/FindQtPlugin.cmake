macro(opaq_process_prl_file_plugin Target prl_file_location Configuration)
    message(STATUS "###### process ${Target} prl: ${prl_file_location}")
    if (NOT EXISTS "${prl_file_location}")
        MESSAGE(FATAL_ERROR "Prl file does not exist ${prl_file_location}")
    endif()

    file(STRINGS "${prl_file_location}" prl_strings REGEX "QMAKE_PRL_LIBS")
    string(REGEX REPLACE "QMAKE_PRL_LIBS *= *([^\n]*)" "\\1" static_depends ${prl_strings} )
    if (${Configuration} STREQUAL RELEASE)
        string(REGEX REPLACE "\\$\\$\\[QT_INSTALL_LIBS\\]" "${CMAKE_PREFIX_PATH}/lib" static_depends "${static_depends}")
    else ()
        string(REGEX REPLACE "\\$\\$\\[QT_INSTALL_LIBS\\]" "${CMAKE_PREFIX_PATH}/debug/lib" static_depends "${static_depends}")
    endif ()
    string(STRIP "${static_depends}" static_depends)
    separate_arguments(static_depends)
    if (APPLE)
        string(REPLACE "-framework;" "-framework " static_depends "${static_depends}")
    endif()
    set_target_properties(${Target} PROPERTIES
        "IMPORTED_LINK_INTERFACE_LIBRARIES_${Configuration}" "${static_depends}"
    )

    message(STATUS "${Target} dependencies: ${static_depends}")
endmacro()

function(find_qtplugin)
    set (oneValueArgs NAME LIBNAME SUBDIR)
    cmake_parse_arguments(find_qtplugin "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    message(STATUS "Find plugin ${find_qtplugin_NAME}")

    find_library(${find_qtplugin_NAME}_LIBRARY NAMES ${find_qtplugin_LIBNAME} HINTS ${CMAKE_PREFIX_PATH}/${find_qtplugin_SUBDIR})
    find_library(${find_qtplugin_NAME}_LIBRARY_DEBUG NAMES ${find_qtplugin_LIBNAME}d HINTS ${CMAKE_PREFIX_PATH}/debug/${find_qtplugin_SUBDIR} ${CMAKE_PREFIX_PATH}/${find_qtplugin_SUBDIR})

    if (MINGW)
        string(REGEX REPLACE "lib(.*)\.(a|lib)$" "\\1.prl" plugin_prl_file "${${find_qtplugin_NAME}_LIBRARY}")
        string(REGEX REPLACE "lib(.*)\.(a|lib)$" "\\1.prl" plugin_prl_file_debug "${${find_qtplugin_NAME}_LIBRARY_DEBUG}")
    else ()
        string(REGEX REPLACE "lib(.*)\.(a|lib)$" "lib\\1.prl" plugin_prl_file "${${find_qtplugin_NAME}_LIBRARY}")
        string(REGEX REPLACE "lib(.*)\.(a|lib)$" "lib\\1.prl" plugin_prl_file_debug "${${find_qtplugin_NAME}_LIBRARY_DEBUG}")
    endif ()

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
        opaq_process_prl_file_plugin(${find_qtplugin_NAME} ${plugin_prl_file} RELEASE)
    endif()

    if (${find_qtplugin_NAME}_LIBRARY_DEBUG)
        set_property(TARGET ${find_qtplugin_NAME} APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(${find_qtplugin_NAME} PROPERTIES IMPORTED_LOCATION_DEBUG "${${find_qtplugin_NAME}_LIBRARY_DEBUG}")
        opaq_process_prl_file_plugin(${find_qtplugin_NAME} ${plugin_prl_file_debug} DEBUG)
    endif()
endfunction()

