if (STATIC_PLUGINS)
    set (PLUGIN_TYPE STATIC)
else ()
    set (PLUGIN_TYPE MODULE)
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../opaq_core)

add_subdirectory(data)
add_subdirectory(forecast)
add_subdirectory(mapping)

set (PLUGINS ${PLUGINS} PARENT_SCOPE)