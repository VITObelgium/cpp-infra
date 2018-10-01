include(FindPackageHandleStandardArgs)

find_path(Sqlpp11_INCLUDE_DIR
    NAMES sqlpp11/sqlpp11.h
    HINTS ${Sqlpp11_INCLUDEDIR}
)

find_path(Sqlpp11_Sqlite3_INCLUDE_DIR
    NAMES sqlpp11/sqlite3/sqlite3.h
    HINTS ${Sqlpp11_INCLUDEDIR}
)

find_path(Sqlpp11_Postgresql_INCLUDE_DIR
    NAMES sqlpp11/postgresql/postgresql.h
    HINTS ${Sqlpp11_INCLUDEDIR}
)

find_library(Sqlpp11_Sqlite3_LIBRARY NAMES sqlpp11-connector-sqlite3 HINTS ${Sqlpp11_ROOT_DIR}/lib)
find_library(Sqlpp11_Sqlite3_LIBRARY_DEBUG NAMES sqlpp11-connector-sqlite3d HINTS ${Sqlpp11_ROOT_DIR}/lib)

find_library(Sqlpp11_Postgresql_LIBRARY NAMES sqlpp11-connector-postgresql HINTS ${Sqlpp11_ROOT_DIR}/lib)
find_library(Sqlpp11_Postgresql_LIBRARY_DEBUG NAMES sqlpp11-connector-postgresqld HINTS ${Sqlpp11_ROOT_DIR}/lib)

message(STATUS "Sqlpp11 sqlite library: ${Sqlpp11_Sqlite3_LIBRARY}")
message(STATUS "Sqlpp11 postgresql library: ${Sqlpp11_Postgresql_LIBRARY}")

find_package_handle_standard_args(Sqlpp11
    FOUND_VAR Sqlpp11_FOUND
    REQUIRED_VARS Sqlpp11_INCLUDE_DIR
)

find_package_handle_standard_args(Sqlpp11_Sqlite3
    FOUND_VAR Sqlpp11_Sqlite3_FOUND
    REQUIRED_VARS Sqlpp11_Sqlite3_INCLUDE_DIR Sqlpp11_Sqlite3_LIBRARY
)

find_package_handle_standard_args(Sqlpp11_Postgresql
    FOUND_VAR Sqlpp11_Postgresql_FOUND
    REQUIRED_VARS Sqlpp11_Postgresql_INCLUDE_DIR Sqlpp11_Postgresql_LIBRARY
)

find_package(sqlite3 CONFIG)
find_package(PostgreSQL)

mark_as_advanced(
    Sqlpp11_ROOT_DIR
    Sqlpp11_INCLUDE_DIR
    Sqlpp11_Sqlite3_INCLUDE_DIR
    Sqlpp11_Sqlite3_LIBRARY
    Sqlpp11_Sqlite3_LIBRARY_DEBUG
    Sqlpp11_Postgresql_INCLUDE_DIR
    Sqlpp11_Postgresql_LIBRARY
    Sqlpp11_Postgresql_LIBRARY_DEBUG
)

if(Sqlpp11_FOUND AND NOT TARGET Sqlpp11::Sqlpp11)
    add_library(Sqlpp11::Sqlpp11 INTERFACE IMPORTED)
    set_target_properties(Sqlpp11::Sqlpp11 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Sqlpp11_INCLUDE_DIR}"
    )
endif()

if(TARGET sqlite3 AND Sqlpp11_Sqlite3_FOUND AND NOT TARGET Sqlpp11::Sqlite3)
    add_library(Sqlpp11::Sqlite3 STATIC IMPORTED)
    set_target_properties(Sqlpp11::Sqlite3 PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        INTERFACE_INCLUDE_DIRECTORIES "${Sqlpp11_Sqlite3_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES sqlite3
        IMPORTED_LOCATION ${Sqlpp11_Sqlite3_LIBRARY}
    )

    if(Sqlpp11_Sqlite3_LIBRARY_DEBUG)
        set_target_properties(Sqlpp11::Sqlite3 PROPERTIES
            IMPORTED_LOCATION_DEBUG "${Sqlpp11_Sqlite3_LIBRARY_DEBUG}"
        )
    endif()
endif()

if(PostgreSQL_FOUND AND Sqlpp11_Postgresql_FOUND AND NOT TARGET Sqlpp11::Postgresql)
    add_library(Sqlpp11::Postgresql STATIC IMPORTED)
    set_target_properties(Sqlpp11::Postgresql PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        INTERFACE_INCLUDE_DIRECTORIES "${Sqlpp11_Postgresql_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES ${PostgreSQL_LIBRARIES}
        IMPORTED_LOCATION ${Sqlpp11_Postgresql_LIBRARY}
    )

    if(Sqlpp11_Postgresql_LIBRARY_DEBUG)
        set_target_properties(Sqlpp11::Postgresql PROPERTIES
            IMPORTED_LOCATION_DEBUG "${Sqlpp11_Postgresql_LIBRARY_DEBUG}"
        )
    endif()
endif()
