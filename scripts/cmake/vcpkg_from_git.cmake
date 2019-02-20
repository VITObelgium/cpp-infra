## # vcpkg_from_git
##
## Checkout a project from a git server
##
## ## Usage:
## ```cmake
## vcpkg_from_git(
##     GIT_URL <https://giturl>
##     OUT_SOURCE_PATH <SOURCE_PATH>
##     [REF <v10.7.3>]
##     [HEAD_REF <master>]
##     [PATCHES <patch1.patch> <patch2.patch>...]
## )
## ```
##
## ## Parameters:
##
## ### GIT_URL
## The full URL of the git server to clone from
##
## ### OUT_SOURCE_PATH
## Specifies the out-variable that will contain the extracted location.
##
## This should be set to `SOURCE_PATH` by convention.
##
## ### REF
## A stable git commit-ish (ideally a tag) that will not change contents. **This should not be a branch.**
##
## For repositories without official releases, this can be set to the full commit id of the current latest master.
##
## ### HEAD_REF
## The unstable git commit-ish (ideally a branch) to pull for `--head` builds.
##
## For most projects, this should be `master`. The chosen branch should be one that is expected to be always buildable on all supported platforms.
##
## Relative paths are based on the port directory.
##
## ## Notes:
## At least one of `REF` and `HEAD_REF` must be specified, however it is preferable for both to be present.
##
## This exports the `VCPKG_HEAD_VERSION` variable during head builds.
##

function(vcpkg_from_git)
    set(zeroValueArgs RECURSE_SUBMODULES)
    set(oneValueArgs OUT_SOURCE_PATH URL REF HEAD_REF)
    set(multipleValuesArgs PATCHES)
    cmake_parse_arguments(_vdud "${zeroValueArgs}" "${oneValueArgs}" "${multipleValuesArgs}" ${ARGN})

    find_program(GIT_EXECUTABLE
        NAMES git
        PATHS "$ENV{ProgramFiles}/Git/bin"
        DOC "Git command line client"
    )
    if (NOT GIT_EXECUTABLE)
        message(FATAL_ERROR "Could not find git binary")
    endif ()

    if(NOT DEFINED _vdud_URL)
        message(FATAL_ERROR "GIT_URL must be specified.")
    endif()

    if (_vdud_RECURSE_SUBMODULES)
        set(RECURSE_ARG --recurse)
    endif ()

    if(NOT DEFINED _vdud_OUT_SOURCE_PATH)
        message(FATAL_ERROR "OUT_SOURCE_PATH must be specified.")
    endif()

    if(NOT DEFINED _vdud_REF AND NOT DEFINED _vdud_HEAD_REF)
        message(FATAL_ERROR "At least one of REF and HEAD_REF must be specified.")
    endif()

    if(VCPKG_USE_HEAD_VERSION AND NOT DEFINED _vdud_HEAD_REF)
        message(STATUS "Package does not specify HEAD_REF. Falling back to non-HEAD version.")
        set(VCPKG_USE_HEAD_VERSION OFF)
    endif()

    set(WORKING_DIRECTORY "${CURRENT_BUILDTREES_DIR}/src")
    if (EXISTS ${WORKING_DIRECTORY})
        file(REMOVE_RECURSE ${WORKING_DIRECTORY})
    endif ()
    file(MAKE_DIRECTORY "${WORKING_DIRECTORY}")

    # Handle --no-head scenarios
    if(NOT VCPKG_USE_HEAD_VERSION)
        if(NOT _vdud_REF)
            message(FATAL_ERROR "Package does not specify REF. It must built using --head.")
        endif()

        string(REPLACE "/" "-" SANITIZED_REF "${_vdud_REF}")
        set(LOGNAME clone-${TARGET_TRIPLET}-${_vdud_REF})
        set(GIT_COMMAND ${GIT_EXECUTABLE} clone ${RECURSE_ARG} --branch ${_vdud_REF} --single-branch ${_vdud_URL})
        if (VCPKG_VERBOSE)
            string(JOIN " " GIT_COMMAND_STRING ${GIT_COMMAND})
            message(STATUS "${GIT_COMMAND_STRING}")
        endif ()

        execute_process(COMMAND ${GIT_COMMAND}
            WORKING_DIRECTORY ${WORKING_DIRECTORY}
            OUTPUT_FILE ${CURRENT_BUILDTREES_DIR}/${LOGNAME}-out.log
            ERROR_FILE ${CURRENT_BUILDTREES_DIR}/${LOGNAME}-err.log
            RESULT_VARIABLE git_error_code
        )
    else ()
        # The following is for --head scenarios
        if(NOT _vdud_REF)
            message(FATAL_ERROR "Package does not specify HEAD_REF.")
        endif()

        set(LOGNAME clone-${TARGET_TRIPLET}-${_vdud_HEAD_REF})
        set(GIT_COMMAND ${GIT_EXECUTABLE} clone ${RECURSE_ARG} --branch ${_vdud_HEAD_REF} --single-branch ${_vdud_URL})
        if (VCPKG_VERBOSE)
            string(JOIN " " GIT_COMMAND_STRING ${GIT_COMMAND})
            message(STATUS "${GIT_COMMAND_STRING}")
        endif ()

        execute_process(COMMAND ${GIT_COMMAND}
            WORKING_DIRECTORY ${WORKING_DIRECTORY}
            OUTPUT_FILE ${CURRENT_BUILDTREES_DIR}/${LOGNAME}-out.log
            ERROR_FILE ${CURRENT_BUILDTREES_DIR}/${LOGNAME}-err.log
            RESULT_VARIABLE git_error_code
        )
    endif()

    file(GLOB REPO_DIR RELATIVE ${WORKING_DIRECTORY} ${WORKING_DIRECTORY}/*)
    list(LENGTH REPO_DIR NUM_DIRS)
    if (NOT NUM_DIRS EQUAL 1)
        message(FATAL_ERROR "Expected a single subdirectory as result of git clone")
    endif ()

    if(git_error_code AND NOT _ap_QUIET)
        message(FATAL_ERROR "Git clone ref '${_vdud_REF}' from '${_vdud_URL}' failed.")
    endif()

    set("${_vdud_OUT_SOURCE_PATH}" ${WORKING_DIRECTORY}/${REPO_DIR} PARENT_SCOPE)    

    # if (_vdud_PATCHES)
    #     message(STATUS "PATCHES ${_vdud_PATCHES}")
    #     vcpkg_apply_patches(
    #         SOURCE_PATH ${OUT_SOURCE_PATH}
    #         PATCHES ${_vdud_PATCHES}
    #     )
    # endif ()

endfunction()
