include(FindPackageHandleStandardArgs)

find_path(Geos_INCLUDE_DIR
    NAMES geos/geos.h geos.h
    HINTS ${Geos_ROOT_DIR}/include ${Geos_INCLUDEDIR}
)

find_path(Geosc_INCLUDE_DIR
    NAMES geos_c.h
    HINTS ${Geos_ROOT_DIR}/include ${Geos_INCLUDEDIR}
)

find_library(Geos_LIBRARY NAMES geos libgeos HINTS ${Geos_ROOT_DIR}/lib)
find_library(Geos_LIBRARY_DEBUG NAMES geosd libgeosd HINTS ${Geos_ROOT_DIR}/lib)

find_library(Geosc_LIBRARY NAMES geos_c HINTS ${Geos_ROOT_DIR}/lib)
find_library(Geosc_LIBRARY_DEBUG NAMES geos_cd HINTS ${Geos_ROOT_DIR}/lib)

message(STATUS "Geos library: ${Geos_LIBRARY}")
message(STATUS "Geos c library: ${Geosc_LIBRARY}")

find_package_handle_standard_args(Geos
    FOUND_VAR Geos_FOUND
    REQUIRED_VARS Geos_INCLUDE_DIR Geosc_INCLUDE_DIR Geos_LIBRARY Geosc_LIBRARY
)

mark_as_advanced(
    Geos_ROOT_DIR
    Geos_INCLUDE_DIR
    Geos_LIBRARY
    Geos_LIBRARY_DEBUG
    Geosc_INCLUDE_DIR
    Geosc_LIBRARY
    Geosc_LIBRARY_DEBUG
)

if(Geos_FOUND AND NOT TARGET Geos::Geos)
    add_library(Geos::Geos STATIC IMPORTED)
    set_target_properties(Geos::Geos PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        INTERFACE_INCLUDE_DIRECTORIES "${Geos_INCLUDE_DIR}"
        IMPORTED_LOCATION ${Geos_LIBRARY}
    )

    if(Geos_LIBRARY_DEBUG)
        set_target_properties(Geos::Geos PROPERTIES
            IMPORTED_LOCATION_DEBUG "${Geos_LIBRARY_DEBUG}"
        )
    endif()
endif()

if(Geos_FOUND AND NOT TARGET Geos::Geosc)
    add_library(Geos::Geosc STATIC IMPORTED)
    set_target_properties(Geos::Geosc PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        INTERFACE_INCLUDE_DIRECTORIES "${Geosc_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES Geos::Geos
        IMPORTED_LOCATION ${Geosc_LIBRARY}
    )

    if(Geosc_LIBRARY_DEBUG)
        set_target_properties(Geos::Geosc PROPERTIES
            IMPORTED_LOCATION_DEBUG "${Geosc_LIBRARY_DEBUG}"
        )
    endif()
endif()
