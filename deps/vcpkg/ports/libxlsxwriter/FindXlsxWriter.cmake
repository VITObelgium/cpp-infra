include(FindPackageHandleStandardArgs)

find_path(XslxWriter_INCLUDE_DIR
    NAMES xlsxwriter.h
    HINTS ${XslxWriter_ROOT_DIR}/include ${XslxWriter_INCLUDEDIR}
)

find_library(XslxWriter_LIBRARY NAMES xlsxwriter HINTS ${XslxWriter_ROOT_DIR}/lib)
find_library(XslxWriter_LIBRARY_DEBUG NAMES xlsxwriterd HINTS ${XslxWriter_ROOT_DIR}/debug/lib)

message(STATUS "XslxWriter library: ${XslxWriter_LIBRARY}")

find_package_handle_standard_args(XslxWriter
    FOUND_VAR XslxWriter_FOUND
    REQUIRED_VARS XslxWriter_INCLUDE_DIR XslxWriter_LIBRARY
)

mark_as_advanced(
    XslxWriter_ROOT_DIR
    XslxWriter_INCLUDE_DIR
    XslxWriter_LIBRARY
    XslxWriter_LIBRARY_DEBUG
)

if(XslxWriter_FOUND AND NOT TARGET XslxWriter::XslxWriter)
    add_library(XslxWriter::XslxWriter STATIC IMPORTED)
    set_target_properties(XslxWriter::XslxWriter PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        INTERFACE_INCLUDE_DIRECTORIES "${XslxWriter_INCLUDE_DIR}"
        IMPORTED_LOCATION ${XslxWriter_LIBRARY}
    )

    if(XslxWriter_LIBRARY_DEBUG)
        set_target_properties(XslxWriter::XslxWriter PROPERTIES
            IMPORTED_LOCATION_DEBUG "${XslxWriter_LIBRARY_DEBUG}"
        )
    endif()
endif()
