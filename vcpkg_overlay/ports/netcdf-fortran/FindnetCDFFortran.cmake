include(FindPackageHandleStandardArgs)

find_path(NetcdfFortran_INCLUDE_DIR
    NAMES netcdf.mod
    HINTS ${NetcdfFortran_ROOT_DIR}/include ${NetcdfFortran_INCLUDEDIR}
)

find_package(netCDF CONFIG REQUIRED QUIET)
find_library(NetcdfFortran_LIBRARY NAMES netcdff HINTS ${NetcdfFortran_ROOT_DIR}/lib)
find_library(NetcdfFortran_LIBRARY_DEBUG NAMES netcdffd netcdff HINTS ${NetcdfFortran_ROOT_DIR}/lib)

find_package_handle_standard_args(NetcdfFortran
    FOUND_VAR NetcdfFortran_FOUND
    REQUIRED_VARS NetcdfFortran_INCLUDE_DIR NetcdfFortran_LIBRARY
)

mark_as_advanced(
    NetcdfFortran_ROOT_DIR
    NetcdfFortran_INCLUDE_DIR
    NetcdfFortran_LIBRARY
    NetcdfFortran_LIBRARY_DEBUG
)

if(NetcdfFortran_FOUND AND NOT TARGET netcdf::fortran)
    add_library(netcdf::fortran STATIC IMPORTED)
    set_target_properties(netcdf::fortran PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${NetcdfFortran_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "netcdf"
        IMPORTED_LOCATION "${NetcdfFortran_LIBRARY}"
    )

    if(NetcdfFortran_LIBRARY_DEBUG)
        set_target_properties(netcdf::fortran PROPERTIES
            IMPORTED_LOCATION_DEBUG "${NetcdfFortran_LIBRARY_DEBUG}"
        )
    endif()
endif()
