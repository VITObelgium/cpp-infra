include(FindPackageHandleStandardArgs)

find_path(Shapelib_INCLUDE_DIR
    NAMES shapefil.h
    HINTS ${Shapelib_ROOT_DIR}/include ${Shapelib_INCLUDEDIR}
)

find_library(Shapelib_LIBRARY NAMES shp HINTS ${Shapelib_ROOT_DIR}/lib)
find_library(Shapelib_LIBRARY_DEBUG NAMES shpd HINTS ${Shapelib_ROOT_DIR}/lib)

message(STATUS "Shapelib library: ${Shapelib_LIBRARY}")

find_package_handle_standard_args(Shapelib
    FOUND_VAR Shapelib_FOUND
    REQUIRED_VARS Shapelib_INCLUDE_DIR Shapelib_LIBRARY
)

mark_as_advanced(
    Shapelib_ROOT_DIR
    Shapelib_INCLUDE_DIR
    Shapelib_LIBRARY
    Shapelib_LIBRARY_DEBUG
)

if(Shapelib_FOUND AND NOT TARGET Shapelib::Shapelib)
    add_library(Shapelib::Shapelib STATIC IMPORTED)
    set_target_properties(Shapelib::Shapelib PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        INTERFACE_INCLUDE_DIRECTORIES "${Shapelib_INCLUDE_DIR}"
        IMPORTED_LOCATION ${Shapelib_LIBRARY}
    )

    if(Shapelib_LIBRARY_DEBUG)
        set_target_properties(Shapelib::Shapelib PROPERTIES
            IMPORTED_LOCATION_DEBUG "${Shapelib_LIBRARY_DEBUG}"
        )
    endif()
endif()
