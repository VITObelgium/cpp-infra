include(FindPackageHandleStandardArgs)

find_path(GMock_INCLUDE_DIR
    NAMES gmock/gmock.h
    HINTS ${GMock_ROOT_DIR}/include ${GMock_INCLUDEDIR}
)

find_library(GMock_LIBRARY NAMES gmock HINTS ${GMock_ROOT_DIR}/lib)
find_library(GMock_LIBRARY_DEBUG NAMES gmockd HINTS ${GMock_ROOT_DIR}/lib)

find_library(GMockMain_LIBRARY NAMES gmock_main HINTS ${GMock_ROOT_DIR}/lib)
find_library(GMockMain_LIBRARY_DEBUG NAMES gmock_maind HINTS ${GMock_ROOT_DIR}/lib)

message(STATUS "GMock library: ${GMock_LIBRARY}")

find_package_handle_standard_args(GMock
    FOUND_VAR GMock_FOUND
    REQUIRED_VARS GMock_INCLUDE_DIR GMock_LIBRARY GMockMain_LIBRARY
)

mark_as_advanced(
    GMock_ROOT_DIR
    GMock_INCLUDE_DIR
    GMock_LIBRARY
    GMock_LIBRARY_DEBUG
    GMockMain_LIBRARY
    GMockMain_LIBRARY_DEBUG
)

if(GMock_FOUND AND NOT TARGET GMock::GMock)
    add_library(GMock::GMock STATIC IMPORTED)
    set_target_properties(GMock::GMock PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        INTERFACE_INCLUDE_DIRECTORIES "${GMock_INCLUDE_DIR}"
        IMPORTED_LOCATION "${GMock_LIBRARY}"
    )

    if(GMock_LIBRARY_DEBUG)
        set_target_properties(GMock::GMock PROPERTIES
            IMPORTED_LOCATION_DEBUG "${GMock_LIBRARY_DEBUG}"
        )
    endif()
endif()

if(GMock_FOUND AND NOT TARGET GMock::Main)
    add_library(GMock::Main STATIC IMPORTED)
    set_target_properties(GMock::Main PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        INTERFACE_INCLUDE_DIRECTORIES "${GMock_INCLUDE_DIR}"
        IMPORTED_LOCATION "${GMockMain_LIBRARY}"
    )

    if(GMockMain_LIBRARY_DEBUG)
        set_target_properties(GMock::Main PROPERTIES
            IMPORTED_LOCATION_DEBUG "${GMockMain_LIBRARY_DEBUG}"
        )
    endif()
endif()
