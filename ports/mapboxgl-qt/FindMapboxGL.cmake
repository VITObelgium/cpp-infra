# Defines the MapboxGL::core target
# Defines the MapboxGL::filesource target
# Defines the MapboxGL::qmapboxgl target
# Defines the MapboxGL::qgeoplugin target

include(FindPackageHandleStandardArgs)

find_path(MapboxGL_INCLUDE_DIR
    NAMES mbgl/style/style.hpp
    HINTS ${MapboxGL_ROOT_DIR}/include ${MapboxGL_INCLUDEDIR}
)

find_library(MapboxGL_core_LIBRARY NAMES mbgl-core HINTS ${MapboxGL_ROOT_DIR}/lib)
find_library(MapboxGL_core_LIBRARY_DEBUG NAMES mbgl-cored HINTS ${MapboxGL_ROOT_DIR}/debug/lib ${MapboxGL_ROOT_DIR}/lib)
message(STATUS "MapboxGL core library: ${MapboxGL_core_LIBRARY}")

find_library(MapboxGL_filesource_LIBRARY NAMES mbgl-filesource HINTS ${MapboxGL_ROOT_DIR}/lib)
find_library(MapboxGL_filesource_LIBRARY_DEBUG NAMES mbgl-filesourced HINTS ${MapboxGL_ROOT_DIR}/debug/lib ${MapboxGL_ROOT_DIR}/lib)
message(STATUS "MapboxGL filesource library: ${MapboxGL_filesource_LIBRARY}")

find_library(MapboxGL_qt_LIBRARY NAMES qmapboxgl HINTS ${MapboxGL_ROOT_DIR}/lib)
find_library(MapboxGL_qt_LIBRARY_DEBUG NAMES qmapboxgld HINTS ${MapboxGL_ROOT_DIR}/debug/lib ${MapboxGL_ROOT_DIR}/lib)
message(STATUS "MapboxGL qt library: ${MapboxGL_qt_LIBRARY_DEBUG}")

find_library(MapboxGL_qgeoplugin_LIBRARY NAMES qgeoservices_mapboxgl HINTS ${MapboxGL_ROOT_DIR}/lib)
find_library(MapboxGL_qgeoplugin_LIBRARY_DEBUG NAMES qgeoservices_mapboxgld HINTS ${MapboxGL_ROOT_DIR}/debug/lib ${MapboxGL_ROOT_DIR}/lib)
message(STATUS "MapboxGL qgeoplugin library: ${MapboxGL_qgeoplugin_LIBRARY_DEBUG}")

find_package(ZLIB REQUIRED QUIET)
find_package(JPEG REQUIRED QUIET)
find_package(PNG REQUIRED QUIET)
find_package(ICU COMPONENTS i18n uc REQUIRED)

find_package(sqlite3 REQUIRED)

find_package_handle_standard_args(MapboxGL
    FOUND_VAR MapboxGL_FOUND
    REQUIRED_VARS
        MapboxGL_INCLUDE_DIR
        MapboxGL_core_LIBRARY
        MapboxGL_filesource_LIBRARY
        ZLIB_FOUND
        JPEG_FOUND
        PNG_FOUND
        ICU_FOUND
        sqlite3_FOUND
)

mark_as_advanced(
    MapboxGL_ROOT_DIR
    MapboxGL_INCLUDE_DIR
    MapboxGL_core_LIBRARY
    MapboxGL_core_LIBRARY_DEBUG
    MapboxGL_filesource_LIBRARY
    MapboxGL_filesource_LIBRARY_DEBUG
    MapboxGL_qt_LIBRARY
    MapboxGL_qt_LIBRARY_DEBUG
    MapboxGL_qgeoplugin_LIBRARY
    MapboxGL_qgeoplugin_LIBRARY_DEBUG
)

if(MapboxGL_FOUND AND NOT TARGET MapboxGL::core)
    add_library(MapboxGL::core STATIC IMPORTED)
    set_target_properties(MapboxGL::core PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        INTERFACE_INCLUDE_DIRECTORIES "${MapboxGL_INCLUDE_DIR}"
        IMPORTED_LOCATION ${MapboxGL_core_LIBRARY}
    )

    if(MapboxGL_core_LIBRARY_DEBUG)
        set_target_properties(MapboxGL::core PROPERTIES
            IMPORTED_LOCATION_DEBUG "${MapboxGL_core_LIBRARY_DEBUG}"
        )
    endif()

    # required dependencies
    set_property(TARGET MapboxGL::core APPEND PROPERTY INTERFACE_LINK_LIBRARIES JPEG::JPEG PNG::PNG ZLIB::ZLIB ICU::i18n ICU::uc)
endif()

if(MapboxGL_FOUND AND NOT TARGET MapboxGL::filesource)
    add_library(MapboxGL::filesource STATIC IMPORTED)
    set_target_properties(MapboxGL::filesource PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        INTERFACE_INCLUDE_DIRECTORIES "${MapboxGL_INCLUDE_DIR}"
        IMPORTED_LOCATION ${MapboxGL_filesource_LIBRARY}
    )

    if(MapboxGL_filesource_LIBRARY_DEBUG)
        set_target_properties(MapboxGL::filesource PROPERTIES
            IMPORTED_LOCATION_DEBUG "${MapboxGL_filesource_LIBRARY_DEBUG}"
        )
    endif()

    # required dependencies
    set_property(TARGET MapboxGL::core APPEND PROPERTY INTERFACE_LINK_LIBRARIES sqlite3)
endif()

if(MapboxGL_qt_LIBRARY AND NOT TARGET MapboxGL::qmapboxgl)
    add_library(MapboxGL::qmapboxgl STATIC IMPORTED)
    set_target_properties(MapboxGL::qmapboxgl PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        INTERFACE_INCLUDE_DIRECTORIES "${MapboxGL_INCLUDE_DIR}"
        IMPORTED_LOCATION ${MapboxGL_qt_LIBRARY}
        INTERFACE_COMPILE_DEFINITIONS QT_MAPBOXGL_STATIC
    )

    if(MapboxGL_qt_LIBRARY_DEBUG)
        set_target_properties(MapboxGL::qmapboxgl PROPERTIES
            IMPORTED_LOCATION_DEBUG "${MapboxGL_qt_LIBRARY_DEBUG}"
        )
    endif()

    # required dependencies
    set_property(TARGET MapboxGL::qmapboxgl APPEND PROPERTY INTERFACE_LINK_LIBRARIES MapboxGL::core MapboxGL::filesource)
endif()

if(MapboxGL_qgeoplugin_LIBRARY AND NOT TARGET MapboxGL::qgeoplugin)
    add_library(MapboxGL::qgeoplugin STATIC IMPORTED)
    set_target_properties(MapboxGL::qgeoplugin PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        IMPORTED_LOCATION ${MapboxGL_qgeoplugin_LIBRARY}
    )

    if(MapboxGL_qgeoplugin_LIBRARY_DEBUG)
        set_target_properties(MapboxGL::qgeoplugin PROPERTIES
            IMPORTED_LOCATION_DEBUG "${MapboxGL_qgeoplugin_LIBRARY_DEBUG}"
        )
    endif()

    # required dependencies
    set_property(TARGET MapboxGL::qgeoplugin APPEND PROPERTY INTERFACE_LINK_LIBRARIES MapboxGL::qmapboxgl)
endif()