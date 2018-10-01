# Defines the Gdal::Gdal target
# Defines the GDAL_DATA_PATH variable containing the path of the gdal data files
# needed for coordinate calculations

include(FindPackageHandleStandardArgs)

find_path(Gdal_INCLUDE_DIR
    NAMES gdal.h
    HINTS ${Gdal_ROOT_DIR}/include ${Gdal_INCLUDEDIR}
)

find_library(Gdal_LIBRARY NAMES gdal HINTS ${Gdal_ROOT_DIR}/lib)
find_library(Gdal_LIBRARY_DEBUG NAMES gdald HINTS ${Gdal_ROOT_DIR}/lib)

foreach(PPATH IN LISTS CMAKE_PREFIX_PATH)
    if (EXISTS ${PPATH}/share/gdal)
        file(TO_CMAKE_PATH ${PPATH}/share/gdal Gdal_DATA_PATH)
    endif ()
endforeach()

message(STATUS "Gdal library: ${Gdal_LIBRARY}")
message(STATUS "Gdal data path: ${Gdal_DATA_PATH}")

find_package(TIFF QUIET)
find_package(EXPAT QUIET)
find_package(ZLIB QUIET)
find_package(PROJ4 QUIET)
find_package(PNG QUIET)
find_package(GIF QUIET)
find_package(Geos QUIET)

if (NOT EMSCRIPTEN)
    find_package(Threads)
endif ()

find_package_handle_standard_args(Gdal
    FOUND_VAR Gdal_FOUND
    REQUIRED_VARS Gdal_INCLUDE_DIR Gdal_LIBRARY ZLIB_FOUND
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

if (TIFF_FOUND)
    set_property(TARGET Gdal::Gdal APPEND PROPERTY INTERFACE_LINK_LIBRARIES TIFF::TIFF)
endif ()

if (EXPAT_FOUND)
    set_property(TARGET Gdal::Gdal APPEND PROPERTY INTERFACE_LINK_LIBRARIES EXPAT::EXPAT)
endif ()

if (ZLIB_FOUND)
    set_property(TARGET Gdal::Gdal APPEND PROPERTY INTERFACE_LINK_LIBRARIES ZLIB::ZLIB)
endif ()

if (PROJ4_FOUND)
    set_property(TARGET Gdal::Gdal APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${PROJ4_LIBRARIES})
endif ()

if (PNG_FOUND)
    set_property(TARGET Gdal::Gdal APPEND PROPERTY INTERFACE_LINK_LIBRARIES PNG::PNG)
endif ()

if (GIF_FOUND)
    set_property(TARGET Gdal::Gdal APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${GIF_LIBRARIES})
endif ()

if (Geos_FOUND)
    set_property(TARGET Gdal::Gdal APPEND PROPERTY INTERFACE_LINK_LIBRARIES Geos::Geosc)
endif ()

if (UNIX AND NOT EMSCRIPTEN)
    set_property(TARGET Gdal::Gdal APPEND PROPERTY INTERFACE_LINK_LIBRARIES Threads::Threads ${CMAKE_DL_LIBS})
endif ()

if (WIN32)
    set_property(TARGET Gdal::Gdal APPEND PROPERTY INTERFACE_LINK_LIBRARIES ws2_32 psapi)
endif ()