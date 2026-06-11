include(FindPackageHandleStandardArgs)
include(CMakeFindDependencyMacro)

find_path(XlsxWriter_INCLUDE_DIR
    NAMES xlsxwriter.h
    HINTS ${XlsxWriter_ROOT_DIR}/include ${_VCPKG_INSTALLED_DIR}/include ${XlsxWriter_INCLUDEDIR}
)

if(DEFINED VCPKG_TARGET_TRIPLET)
    find_library(XlsxWriter_LIBRARY NAMES xlsxwriter PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH)
    find_library(XlsxWriter_LIBRARY_DEBUG NAMES xlsxwriter PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH)
    find_dependency(MINIZIP NAMES unofficial-minizip CONFIG REQUIRED)
else ()
    find_library(XlsxWriter_LIBRARY NAMES xlsxwriter)
    find_library(Minizip_LIBRARY NAMES minizip)
    find_dependency(OpenSSL)
endif ()

message(STATUS "XlsxWriter library: Rel ${XlsxWriter_LIBRARY} Dbg ${XlsxWriter_LIBRARY_DEBUG}")

find_package_handle_standard_args(XlsxWriter
    FOUND_VAR XlsxWriter_FOUND
    REQUIRED_VARS XlsxWriter_INCLUDE_DIR XlsxWriter_LIBRARY
)

mark_as_advanced(
    XlsxWriter_ROOT_DIR
    XlsxWriter_INCLUDE_DIR
    XlsxWriter_LIBRARY
    XlsxWriter_LIBRARY_DEBUG
)

if(XlsxWriter_FOUND AND NOT TARGET XlsxWriter::XlsxWriter)
    add_library(XlsxWriter::XlsxWriter STATIC IMPORTED)
    set_target_properties(XlsxWriter::XlsxWriter PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        INTERFACE_INCLUDE_DIRECTORIES "${XlsxWriter_INCLUDE_DIR}"
        IMPORTED_LOCATION ${XlsxWriter_LIBRARY}
    )

    if(DEFINED VCPKG_TARGET_TRIPLET)
        set_target_properties(XlsxWriter::XlsxWriter PROPERTIES
            INTERFACE_LINK_LIBRARIES "unofficial::minizip::minizip"
        )
    else()
        set_target_properties(XlsxWriter::XlsxWriter PROPERTIES
            INTERFACE_LINK_LIBRARIES "${Minizip_LIBRARY};$<TARGET_NAME_IF_EXISTS:OpenSSL::SSL>"
        )
    endif ()

    if(XlsxWriter_LIBRARY_DEBUG)
        set_target_properties(XlsxWriter::XlsxWriter PROPERTIES
            IMPORTED_LOCATION_DEBUG "${XlsxWriter_LIBRARY_DEBUG}"
        )
    endif()
endif()
