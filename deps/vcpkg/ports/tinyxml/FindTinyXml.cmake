include(FindPackageHandleStandardArgs)

find_path(TinyXml_INCLUDE_DIR
    NAMES tinyxml.h
    HINTS ${TinyXml_ROOT_DIR}/include ${TinyXml_INCLUDEDIR}
)

find_library(TinyXml_LIBRARY NAMES tinyxml HINTS ${TinyXml_ROOT_DIR}/lib)
find_library(TinyXml_LIBRARY_DEBUG NAMES tinyxmld HINTS ${TinyXml_ROOT_DIR}/lib)

message(STATUS "TinyXml library: ${TinyXml_LIBRARY}")

find_package_handle_standard_args(TinyXml
    FOUND_VAR TinyXml_FOUND
    REQUIRED_VARS TinyXml_INCLUDE_DIR TinyXml_LIBRARY
)

mark_as_advanced(
    TinyXml_ROOT_DIR
    TinyXml_INCLUDE_DIR
    TinyXml_LIBRARY
    TinyXml_LIBRARY_DEBUG
)

if(TinyXml_FOUND AND NOT TARGET TinyXml::TinyXml)
    add_library(TinyXml::TinyXml STATIC IMPORTED)
    set_target_properties(TinyXml::TinyXml PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        INTERFACE_INCLUDE_DIRECTORIES "${TinyXml_INCLUDE_DIR}"
        INTERFACE_COMPILE_DEFINITIONS TIXML_USE_STL
        IMPORTED_LOCATION ${TinyXml_LIBRARY}
    )

    if(TinyXml_LIBRARY_DEBUG)
        set_target_properties(TinyXml::TinyXml PROPERTIES
            IMPORTED_LOCATION_DEBUG "${TinyXml_LIBRARY_DEBUG}"
        )
    endif()
endif()
