include(FindPackageHandleStandardArgs)

set(_Sqlite3_TARGET unofficial::sqlite3::sqlite3)

find_package(unofficial-sqlite3 CONFIG QUIET)

function(_sqlite3_get_target_property output target property)
  get_target_property(_sqlite3_value "${target}" "${property}")
  if(_sqlite3_value AND NOT _sqlite3_value MATCHES "-NOTFOUND$")
    set("${output}" "${_sqlite3_value}" PARENT_SCOPE)
  endif()
endfunction()

if(TARGET ${_Sqlite3_TARGET})
  _sqlite3_get_target_property(Sqlite3_INCLUDE_DIR "${_Sqlite3_TARGET}" INTERFACE_INCLUDE_DIRECTORIES)
  _sqlite3_get_target_property(Sqlite3_LIBRARY_RELEASE "${_Sqlite3_TARGET}" IMPORTED_LOCATION_RELEASE)
  _sqlite3_get_target_property(Sqlite3_LIBRARY_DEBUG "${_Sqlite3_TARGET}" IMPORTED_LOCATION_DEBUG)
  _sqlite3_get_target_property(Sqlite3_LIBRARY "${_Sqlite3_TARGET}" IMPORTED_LOCATION)
  _sqlite3_get_target_property(_Sqlite3_LINK_LIBRARIES "${_Sqlite3_TARGET}" INTERFACE_LINK_LIBRARIES)

  if(Sqlite3_INCLUDE_DIR)
    list(GET Sqlite3_INCLUDE_DIR 0 Sqlite3_INCLUDE_DIR)
  endif()
else()
  if(DEFINED VCPKG_TARGET_TRIPLET AND DEFINED _VCPKG_INSTALLED_DIR)
    set(_Sqlite3_VCPKG_ROOT "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")

    find_path(Sqlite3_INCLUDE_DIR
            NAMES sqlite3.h
            PATHS "${_Sqlite3_VCPKG_ROOT}/include"
            NO_DEFAULT_PATH
        )

    find_library(Sqlite3_LIBRARY_RELEASE
            NAMES sqlite3 sqlite
            PATHS "${_Sqlite3_VCPKG_ROOT}/lib"
            NO_DEFAULT_PATH
        )

    find_library(Sqlite3_LIBRARY_DEBUG
            NAMES sqlite3 sqlite3d sqlited sqlite
            PATHS "${_Sqlite3_VCPKG_ROOT}/debug/lib"
            NO_DEFAULT_PATH
        )
  endif()

  find_path(Sqlite3_INCLUDE_DIR
        NAMES sqlite3.h
        HINTS ${Sqlite3_ROOT_DIR}/include ${Sqlite3_INCLUDEDIR}
    )

  find_library(Sqlite3_LIBRARY_RELEASE
        NAMES sqlite3 sqlite
        HINTS ${Sqlite3_ROOT_DIR}/lib
    )

  if(NOT Sqlite3_LIBRARY_RELEASE)
    find_library(Sqlite3_LIBRARY
            NAMES sqlite3 sqlite
            HINTS ${Sqlite3_ROOT_DIR}/lib
        )
  endif()
endif()

if(Sqlite3_LIBRARY_RELEASE)
  set(Sqlite3_LIBRARY "${Sqlite3_LIBRARY_RELEASE}")
elseif(Sqlite3_LIBRARY_DEBUG)
  set(Sqlite3_LIBRARY "${Sqlite3_LIBRARY_DEBUG}")
endif()

if(Sqlite3_INCLUDE_DIR AND EXISTS "${Sqlite3_INCLUDE_DIR}/sqlite3.h")
  file(STRINGS "${Sqlite3_INCLUDE_DIR}/sqlite3.h" _Sqlite3_VERSION_LINE
        REGEX "^#define SQLITE_VERSION  *\"[0-9]+\\.[0-9]+\\.[0-9]+\""
        LIMIT_COUNT 1
    )
  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" Sqlite3_VERSION "${_Sqlite3_VERSION_LINE}")
  unset(_Sqlite3_VERSION_LINE)
endif()

set(Sqlite3_INCLUDE_DIRS "${Sqlite3_INCLUDE_DIR}")
set(Sqlite3_LIBRARIES "")

if(Sqlite3_LIBRARY_RELEASE AND Sqlite3_LIBRARY_DEBUG)
  list(APPEND Sqlite3_LIBRARIES
        optimized "${Sqlite3_LIBRARY_RELEASE}"
        debug "${Sqlite3_LIBRARY_DEBUG}"
    )
elseif(Sqlite3_LIBRARY_RELEASE)
  list(APPEND Sqlite3_LIBRARIES "${Sqlite3_LIBRARY_RELEASE}")
elseif(Sqlite3_LIBRARY_DEBUG)
  list(APPEND Sqlite3_LIBRARIES "${Sqlite3_LIBRARY_DEBUG}")
elseif(Sqlite3_LIBRARY)
  list(APPEND Sqlite3_LIBRARIES "${Sqlite3_LIBRARY}")
endif()

if(_Sqlite3_LINK_LIBRARIES)
  list(APPEND Sqlite3_LIBRARIES ${_Sqlite3_LINK_LIBRARIES})
endif()

set(SQLite3_INCLUDE_DIR "${Sqlite3_INCLUDE_DIR}")
set(SQLite3_INCLUDE_DIRS "${Sqlite3_INCLUDE_DIRS}")
set(SQLite3_LIBRARY "${Sqlite3_LIBRARY}")
set(SQLite3_LIBRARY_RELEASE "${Sqlite3_LIBRARY_RELEASE}")
set(SQLite3_LIBRARY_DEBUG "${Sqlite3_LIBRARY_DEBUG}")
set(SQLite3_LIBRARIES "${Sqlite3_LIBRARIES}")
set(SQLite3_VERSION "${Sqlite3_VERSION}")

if(CMAKE_FIND_PACKAGE_NAME STREQUAL "SQLite3")
  find_package_handle_standard_args(SQLite3
        REQUIRED_VARS SQLite3_INCLUDE_DIR SQLite3_LIBRARY
        VERSION_VAR SQLite3_VERSION
    )
  set(Sqlite3_FOUND "${SQLite3_FOUND}")
else()
  find_package_handle_standard_args(Sqlite3
        REQUIRED_VARS Sqlite3_INCLUDE_DIR Sqlite3_LIBRARY
        VERSION_VAR Sqlite3_VERSION
    )
  set(SQLite3_FOUND "${Sqlite3_FOUND}")
endif()

if(Sqlite3_FOUND OR SQLite3_FOUND)
  if(TARGET ${_Sqlite3_TARGET})
    foreach(_Sqlite3_COMPAT_TARGET IN ITEMS SQLite::SQLite3 SQLite3::SQLite3 Sqlite3::Sqlite3 sqlite3)
      if(NOT TARGET ${_Sqlite3_COMPAT_TARGET})
        add_library(${_Sqlite3_COMPAT_TARGET} INTERFACE IMPORTED)
        set_target_properties(${_Sqlite3_COMPAT_TARGET} PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${Sqlite3_INCLUDE_DIRS}"
                    INTERFACE_LINK_LIBRARIES "${_Sqlite3_TARGET}"
                )
      endif()
    endforeach()
  else()
    if(NOT TARGET SQLite::SQLite3)
      add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
      set_target_properties(SQLite::SQLite3 PROPERTIES
                IMPORTED_LINK_INTERFACE_LANGUAGES "C"
                INTERFACE_INCLUDE_DIRECTORIES "${Sqlite3_INCLUDE_DIRS}"
            )

      if(Sqlite3_LIBRARY)
        set_target_properties(SQLite::SQLite3 PROPERTIES
                    IMPORTED_LOCATION "${Sqlite3_LIBRARY}"
                )
      endif()

      if(Sqlite3_LIBRARY_RELEASE)
        set_property(TARGET SQLite::SQLite3 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(SQLite::SQLite3 PROPERTIES
                    IMPORTED_LOCATION_RELEASE "${Sqlite3_LIBRARY_RELEASE}"
                    MAP_IMPORTED_CONFIG_MINSIZEREL Release
                    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
                )
      endif()

      if(Sqlite3_LIBRARY_DEBUG)
        set_property(TARGET SQLite::SQLite3 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(SQLite::SQLite3 PROPERTIES
                    IMPORTED_LOCATION_DEBUG "${Sqlite3_LIBRARY_DEBUG}"
                )
      endif()
    endif()

    foreach(_Sqlite3_COMPAT_TARGET IN ITEMS SQLite3::SQLite3 Sqlite3::Sqlite3 sqlite3)
      if(NOT TARGET ${_Sqlite3_COMPAT_TARGET})
        add_library(${_Sqlite3_COMPAT_TARGET} INTERFACE IMPORTED)
        set_target_properties(${_Sqlite3_COMPAT_TARGET} PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${Sqlite3_INCLUDE_DIRS}"
                    INTERFACE_LINK_LIBRARIES SQLite::SQLite3
                )
      endif()
    endforeach()
  endif()
endif()

mark_as_advanced(
    Sqlite3_ROOT_DIR
    Sqlite3_INCLUDE_DIR
    Sqlite3_LIBRARY
    Sqlite3_LIBRARY_RELEASE
    Sqlite3_LIBRARY_DEBUG
    SQLite3_INCLUDE_DIR
    SQLite3_LIBRARY
    SQLite3_LIBRARY_RELEASE
    SQLite3_LIBRARY_DEBUG
)

unset(_Sqlite3_COMPAT_TARGET)
unset(_Sqlite3_LINK_LIBRARIES)
unset(_Sqlite3_TARGET)
unset(_Sqlite3_VCPKG_ROOT)
