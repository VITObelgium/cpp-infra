## # vcpkg_build_autotools
##
## Build an autoconf project.
##
## ## Usage:
## ```cmake
## vcpkg_build_autotools([DISABLE_PARALLEL] [TARGET <target>])
## ```
##
## ## Parameters:
## ### DISABLE_PARALLEL
## The underlying buildsystem will be instructed to not parallelize
##
## ### TARGET
## The target passed to the automake build command (`make <target>`). If not specified, no target will
## be passed.
##
## ### ADD_BIN_TO_PATH
## Adds the appropriate Release and Debug `bin\` directories to the path during the build such that executables can run against the in-tree DLLs.
##
## ## Notes:
## This command should be preceeded by a call to [`vcpkg_configure_autoconf()`](vcpkg_configure_autoconf.md).
## You can use the alias [`vcpkg_install_autoconf()`](vcpkg_configure_autoconf.md) function if your automake script supports the
## "install" target
##

include(ProcessorCount)
ProcessorCount(NUM_CORES)

function(vcpkg_build_autotools)
    cmake_parse_arguments(_bc "DISABLE_PARALLEL;ADD_BIN_TO_PATH;IN_SOURCE" "PARALLEL_JOBS;TARGET;LOGFILE_ROOT;BUILD_TOOL" "" ${ARGN})

    if(NOT _bc_LOGFILE_ROOT)
        set(_bc_LOGFILE_ROOT "build")
    endif()

    if(NOT _bc_BUILD_TOOL)
        set(_bc_BUILD_TOOL "make")
    endif()

    find_program(BUILD_TOOL ${_bc_BUILD_TOOL})
    if (NOT BUILD_TOOL)
        message(FATAL_ERROR "Failed to locate build tool: ${_bc_BUILD_TOOL}")
    endif ()

    set(PARALLEL_ARG "-j${NUM_CORES}")
    set(NO_PARALLEL_ARG "-j1")

    if(_bc_TARGET)
        set(TARGET_PARAM ${_bc_TARGET})
    else()
        set(TARGET_PARAM)
    endif()

    if (TARGET_PARAM STREQUAL "install")
        set (STATUS_MESSAGE "Install")
    else ()
        set (STATUS_MESSAGE "Building")
    endif ()

    if(_bc_DISABLE_PARALLEL)
        set(PARALLEL_ARG ${NO_PARALLEL_ARG})
    elseif (_bc_PARALLEL_JOBS)
        set(PARALLEL_ARG "-j${_bc_PARALLEL_JOBS}")
    endif()

    if (VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        set(MAKE_WRAPPER emmake)
    endif ()

    foreach(BUILDTYPE "debug" "release")
        if(NOT DEFINED VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL BUILDTYPE)
            if(BUILDTYPE STREQUAL "debug")
                set(SHORT_BUILDTYPE "dbg")
                set(CONFIG "Debug")
            else()
                set(SHORT_BUILDTYPE "rel")
                set(CONFIG "Release")
            endif()

            message(STATUS "${STATUS_MESSAGE} ${TARGET_TRIPLET}-${SHORT_BUILDTYPE}")
            set(LOGPREFIX "${CURRENT_BUILDTREES_DIR}/${_bc_LOGFILE_ROOT}-${TARGET_TRIPLET}-${SHORT_BUILDTYPE}")
            set(LOGS)

            if(_bc_ADD_BIN_TO_PATH)
                set(_BACKUP_ENV_PATH "$ENV{PATH}")
                if(BUILDTYPE STREQUAL "debug")
                    set(ENV{PATH} "${CURRENT_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin;$ENV{PATH}")
                else()
                    set(ENV{PATH} "${CURRENT_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin;$ENV{PATH}")
                endif()
            endif()

            if (_bc_IN_SOURCE)
                if(BUILDTYPE STREQUAL "debug")
                    set (WORKING_DIR ${SOURCE_PATH_DEBUG})
                else()
                    set (WORKING_DIR ${SOURCE_PATH_RELEASE})
                endif()
            else ()
                set (WORKING_DIR ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-${SHORT_BUILDTYPE})
            endif ()

            execute_process(
                COMMAND ${MAKE_WRAPPER} ${BUILD_TOOL} ${PARALLEL_ARG} ${TARGET_PARAM}
                OUTPUT_FILE "${LOGPREFIX}-out.log"
                ERROR_FILE "${LOGPREFIX}-err.log"
                RESULT_VARIABLE error_code
                WORKING_DIRECTORY ${WORKING_DIR})
            if(error_code)
                file(READ "${LOGPREFIX}-out.log" out_contents)
                file(READ "${LOGPREFIX}-err.log" err_contents)

                if(out_contents)
                    list(APPEND LOGS "${LOGPREFIX}-out.log")
                endif()
                if(err_contents)
                    list(APPEND LOGS "${LOGPREFIX}-err.log")
                endif()

                if(error_code)
                    set(STRINGIFIED_LOGS)
                    foreach(LOG ${LOGS})
                        file(TO_NATIVE_PATH "${LOG}" NATIVE_LOG)
                        list(APPEND STRINGIFIED_LOGS "    ${NATIVE_LOG}\n")
                    endforeach()
                    set(_eb_COMMAND ${MAKE_WRAPPER} make ${PARALLEL_ARG} ${TARGET_PARAM})
                    set(_eb_WORKING_DIRECTORY ${WORKING_DIR})
                    message(FATAL_ERROR
                        "  Command failed: ${_eb_COMMAND}\n"
                        "  Working Directory: ${_eb_WORKING_DIRECTORY}\n"
                        "  See logs for more information:\n"
                        ${STRINGIFIED_LOGS})
                endif()
            endif()
            if(_bc_ADD_BIN_TO_PATH)
                set(ENV{PATH} "${_BACKUP_ENV_PATH}")
            endif()
        endif()
    endforeach()
endfunction()
