# Defines the Gdal::Gdal target
# Defines the GDAL_DATA_PATH variable containing the path of the gdal data files
# needed for coordinate calculations

include(FindPackageHandleStandardArgs)

find_path(Gdal_INCLUDE_DIR
    NAMES gdal.h
    HINTS ${Gdal_ROOT_DIR}/include ${Gdal_INCLUDEDIR}
)

find_library(Gdal_LIBRARY NAMES gdal gdal_i HINTS ${Gdal_ROOT_DIR}/lib)
find_library(Gdal_LIBRARY_DEBUG NAMES gdald HINTS ${Gdal_ROOT_DIR}/lib)

foreach(PPATH IN LISTS CMAKE_PREFIX_PATH)
    if (EXISTS ${PPATH}/share/gdal)
        file(TO_CMAKE_PATH ${PPATH}/share/gdal Gdal_DATA_PATH)
    endif ()
endforeach()

message(STATUS "Gdal library: ${Gdal_LIBRARY}")
message(STATUS "Gdal data path: ${Gdal_DATA_PATH}")

if (NOT EMSCRIPTEN)
    find_package(Threads)
endif ()

find_package_handle_standard_args(Gdal
    FOUND_VAR Gdal_FOUND
    REQUIRED_VARS Gdal_INCLUDE_DIR Gdal_LIBRARY
)

mark_as_advanced(
    Gdal_ROOT_DIR
    Gdal_INCLUDE_DIR
    Gdal_LIBRARY
    Gdal_LIBRARY_DEBUG
)

if(Gdal_FOUND AND NOT TARGET Gdal::Gdal)
    add_library(Gdal::Gdal STATIC IMPORTED)
    set_target_properties(Gdal::Gdal PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        INTERFACE_INCLUDE_DIRECTORIES "${Gdal_INCLUDE_DIR}"
        INTERFACE_COMPILE_DEFINITIONS CPL_DISABLE_DLL
        IMPORTED_LOCATION ${Gdal_LIBRARY}
    )

    if(Gdal_LIBRARY_DEBUG)
        set_target_properties(Gdal::Gdal PROPERTIES
            IMPORTED_LOCATION_DEBUG "${Gdal_LIBRARY_DEBUG}"
        )
    endif()
endif()
