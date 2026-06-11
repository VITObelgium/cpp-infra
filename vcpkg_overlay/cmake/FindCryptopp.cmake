include(FindPackageHandleStandardArgs)
include(CMakeFindDependencyMacro)

find_path(Cryptopp_INCLUDE_DIR
    NAMES cryptopp/cryptlib.h
    HINTS ${Cryptopp_ROOT_DIR}/include ${_VCPKG_INSTALLED_DIR}/include ${Cryptopp_INCLUDEDIR}
)

if(DEFINED VCPKG_TARGET_TRIPLET)
    find_library(Cryptopp_LIBRARY NAMES cryptopp PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH)
    find_library(Cryptopp_LIBRARY_DEBUG NAMES cryptopp PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH)
else ()
    find_library(Cryptopp_LIBRARY NAMES cryptopp)
endif ()
# find_dependency(MINIZIP NAMES unofficial-minizip CONFIG REQUIRED)

message(STATUS "Cryptopp library: Rel ${Cryptopp_LIBRARY} Dbg ${Cryptopp_LIBRARY_DEBUG}")

find_package_handle_standard_args(Cryptopp
    FOUND_VAR Cryptopp_FOUND
    REQUIRED_VARS Cryptopp_INCLUDE_DIR Cryptopp_LIBRARY
)

mark_as_advanced(
    Cryptopp_ROOT_DIR
    Cryptopp_INCLUDE_DIR
    Cryptopp_LIBRARY
    Cryptopp_LIBRARY_DEBUG
)

if(Cryptopp_FOUND AND NOT TARGET cryptopp::cryptopp)
    add_library(cryptopp::cryptopp UNKNOWN IMPORTED)
    set_target_properties(cryptopp::cryptopp PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        INTERFACE_INCLUDE_DIRECTORIES "${Cryptopp_INCLUDE_DIR}"
        IMPORTED_LOCATION ${Cryptopp_LIBRARY}
    )

    if(Cryptopp_LIBRARY_DEBUG)
        set_target_properties(cryptopp::cryptopp PROPERTIES
            IMPORTED_LOCATION_DEBUG "${Cryptopp_LIBRARY_DEBUG}"
        )
    endif()
endif()
