## # vcpkg_configure_autoconf
##
## Configure CMake for Debug and Release builds of a project.
##
## ## Usage
## ```cmake
## vcpkg_configure_autoconf(
##     SOURCE_PATH <${SOURCE_PATH}>
##     [OPTIONS <--option-for-all-builds>...]
##     [OPTIONS_RELEASE <--option-for-release>...]
##     [OPTIONS_DEBUG <--option-for-debug>...]
## )
## ```
##
## ## Parameters
## ### SOURCE_PATH
## Specifies the directory containing the configure script. By convention, this is usually set in the portfile as the variable `SOURCE_PATH`.
##
## ### OPTIONS
## Additional options passed to autoconf during the configuration.
##
## ### OPTIONS_RELEASE
## Additional options passed to autoconf during the Release configuration. These are in addition to `OPTIONS`.
##
## ### OPTIONS_DEBUG
## Additional options passed to autoconf during the Debug configuration. These are in addition to `OPTIONS`.
##
## ## Notes
## This command supplies many common arguments to autoconf. To see the full list, examine the source.
##
set (_csc_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "")

function(vcpkg_configure_autoconf)
    cmake_parse_arguments(_csc "IN_SOURCE" "SOURCE_PATH;SOURCE_PATH_DEBUG;SOURCE_PATH_RELEASE;CXX_STANDARD" "OPTIONS;OPTIONS_DEBUG;OPTIONS_RELEASE" ${ARGN})

    if(DEFINED ENV{PROCESSOR_ARCHITEW6432})
        set(_csc_HOST_ARCHITECTURE $ENV{PROCESSOR_ARCHITEW6432})
    else()
        set(_csc_HOST_ARCHITECTURE $ENV{PROCESSOR_ARCHITECTURE})
    endif()

    file(REMOVE_RECURSE ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg)

    if((NOT DEFINED VCPKG_CXX_FLAGS_DEBUG AND NOT DEFINED VCPKG_C_FLAGS_DEBUG) OR
        (DEFINED VCPKG_CXX_FLAGS_DEBUG AND DEFINED VCPKG_C_FLAGS_DEBUG))
    else()
        message(FATAL_ERROR "You must set both the VCPKG_CXX_FLAGS_DEBUG and VCPKG_C_FLAGS_DEBUG")
    endif()
    if((NOT DEFINED VCPKG_CXX_FLAGS_RELEASE AND NOT DEFINED VCPKG_C_FLAGS_RELEASE) OR
        (DEFINED VCPKG_CXX_FLAGS_RELEASE AND DEFINED VCPKG_C_FLAGS_RELEASE))
    else()
        message(FATAL_ERROR "You must set both the VCPKG_CXX_FLAGS_RELEASE and VCPKG_C_FLAGS_RELEASE")
    endif()
    if((NOT DEFINED VCPKG_CXX_FLAGS AND NOT DEFINED VCPKG_C_FLAGS) OR
        (DEFINED VCPKG_CXX_FLAGS AND DEFINED VCPKG_C_FLAGS))
    else()
        message(FATAL_ERROR "You must set both the VCPKG_CXX_FLAGS and VCPKG_C_FLAGS")
    endif()

    if (NOT CMAKE_HOST_SYSTEM_NAME STREQUAL VCPKG_CMAKE_SYSTEM_NAME)
        set (CMAKE_CROSSCOMPILING ON)
    endif ()

    if (VCPKG_CHAINLOAD_TOOLCHAIN_FILE)
        include (${VCPKG_CHAINLOAD_TOOLCHAIN_FILE})
    elseif(VCPKG_TARGET_TRIPLET)
        # include the triplet cmake toolchain file if it is present unless a chainload toolchain was provided
        include(${CMAKE_CURRENT_LIST_DIR}/../../triplets/${VCPKG_TARGET_TRIPLET}.cmake OPTIONAL)
    endif ()

    vcpkg_assemble_compiler_cflags(DEBUG _ac_CFLAGS_DEB RELEASE _ac_CFLAGS_REL)
    vcpkg_assemble_compiler_cxxflags(DEBUG _ac_CXXFLAGS_DEB RELEASE _ac_CXXFLAGS_REL STANDARD ${_csc_CXX_STANDARD})

    if (CMAKE_CROSSCOMPILING OR HOST MATCHES ".*-musl")
        message(STATUS "CROSS COMPILING on host '${HOST}'")
        set(HOST_ARG --host=${HOST})
    endif ()

    if (${VCPKG_LIBRARY_LINKAGE} STREQUAL "static")
        list(APPEND _csc_OPTIONS --disable-shared)
        list(APPEND _csc_OPTIONS --enable-static)
    endif ()

    if (VCPKG_VERBOSE)
        message(STATUS "PIC: ${CMAKE_POSITION_INDEPENDENT_CODE}")
        message(STATUS "Visibility C: ${CMAKE_C_VISIBILITY_PRESET}")
        message(STATUS "Visibility C++: ${CMAKE_CXX_VISIBILITY_PRESET}")
    endif ()

    if (CMAKE_EXE_LINKER_FLAGS)
        STRING(JOIN " " EXPORT_LDFLAGS ${CMAKE_EXE_LINKER_FLAGS})
    endif ()

    if(NOT DEFINED VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL "debug")
        if (_csc_SOURCE_PATH_DEBUG)
            set (SOURCE_DIR ${_csc_SOURCE_PATH_DEBUG})
        elseif(_csc_SOURCE_PATH)
            set (SOURCE_DIR ${_csc_SOURCE_PATH})
        else ()
            message (FATAL_ERROR "No valid debug SOURCE_PATH provided")
        endif ()

        if (_csc_IN_SOURCE)
            set (WORKING_DIR ${SOURCE_DIR})
        else ()
            set (WORKING_DIR ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg)
        endif ()

        STRING(JOIN " " EXPORT_CFLAGS ${_ac_CFLAGS_DEB})
        STRING(JOIN " " EXPORT_CXXFLAGS ${_ac_CXXFLAGS_DEB})
        set(EXPORT_CPPFLAGS "${EXPORT_CFLAGS}")
        set(EXPORT_LDFLAGS "${EXPORT_LDFLAGS}")
        configure_file(${_csc_CURRENT_LIST_DIR}/runconfigure.sh.in ${WORKING_DIR}/runconfigure.sh)

        if (VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
            set(command
                emconfigure ${SOURCE_DIR}/configure
                "${_csc_OPTIONS}"
                "${_csc_OPTIONS_DEBUG}"
                --enable-debug
                --disable-dependency-tracking
                --prefix=${CURRENT_PACKAGES_DIR}/debug
            )
        else ()
            set(command
                sh ${WORKING_DIR}/runconfigure.sh
                "${_csc_OPTIONS}"
                "${_csc_OPTIONS_DEBUG}"
                ${HOST_ARG}
                --enable-debug
                --disable-dependency-tracking
                --prefix=${CURRENT_PACKAGES_DIR}/debug
            )
        endif ()

        message(STATUS "Configuring ${TARGET_TRIPLET}-dbg")
        if (VCPKG_VERBOSE)
            STRING(JOIN " " COMMAND_STRING "${command}")
            message(STATUS "Autoconf deb cmd ${COMMAND_STRING}")
            message(STATUS "CFLAGS: ${EXPORT_CFLAGS}")
            message(STATUS "CXXFLAGS: ${EXPORT_CXXFLAGS}")
            message(STATUS "LDFLAGS: ${EXPORT_LDFLAGS}")
        endif ()
        file(MAKE_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg)

        vcpkg_execute_required_process(
            COMMAND ${command}
            WORKING_DIRECTORY ${WORKING_DIR}
            LOGNAME config-${TARGET_TRIPLET}-dbg
        )
    endif()

    if(NOT DEFINED VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL "release")
        if (_csc_SOURCE_PATH_RELEASE)
            set (SOURCE_DIR ${_csc_SOURCE_PATH_RELEASE})
        elseif(_csc_SOURCE_PATH)
            set (SOURCE_DIR ${_csc_SOURCE_PATH})
        else ()
            message (FATAL_ERROR "No valid release SOURCE_PATH provided")
        endif ()

        if (_csc_IN_SOURCE)
            set (WORKING_DIR ${SOURCE_DIR})
        else ()
            set (WORKING_DIR ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel)
        endif ()

        STRING(JOIN " " EXPORT_CFLAGS ${_ac_CFLAGS_REL})
        STRING(JOIN " " EXPORT_CXXFLAGS ${_ac_CXXFLAGS_REL})
        set(EXPORT_CPPFLAGS "${EXPORT_CFLAGS}")
        configure_file(${_csc_CURRENT_LIST_DIR}/runconfigure.sh.in ${WORKING_DIR}/runconfigure.sh)

        if (VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
            set(command
                emconfigure ${SOURCE_DIR}/configure
                    ${HOST_ARG}
                    "${_csc_OPTIONS}"
                    "${_csc_OPTIONS_RELEASE}"
                    --prefix=${CURRENT_PACKAGES_DIR}
            )
        else ()
            set(command
                sh ${WORKING_DIR}/runconfigure.sh
                    ${HOST_ARG}
                    "${_csc_OPTIONS}"
                    "${_csc_OPTIONS_RELEASE}"
                    --prefix=${CURRENT_PACKAGES_DIR}
            )
        endif ()

        message(STATUS "Configuring ${TARGET_TRIPLET}-rel")
        if (VCPKG_VERBOSE)
            STRING(JOIN " " COMMAND_STRING ${command})
            message(STATUS "Autoconf rel cmd ${COMMAND_STRING}")
            message(STATUS "CFLAGS: ${EXPORT_CFLAGS}")
            message(STATUS "CXXFLAGS: ${EXPORT_CXXFLAGS}")
            message(STATUS "LDFLAGS: ${EXPORT_LDFLAGS}")
        endif ()
        file(MAKE_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel)

        vcpkg_execute_required_process(
            COMMAND ${command}
            WORKING_DIRECTORY ${WORKING_DIR}
            LOGNAME config-${TARGET_TRIPLET}-rel
        )
    endif()
endfunction()
