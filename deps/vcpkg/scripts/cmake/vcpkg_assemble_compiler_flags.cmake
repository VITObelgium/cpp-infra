#.rst:
# .. command:: vcpkg_assemble_compiler_flags
#
#  Create compiler flags based on the current cmake toolchain
#  to pass to other build systems
#
#  ::
#  vcpkg_assemble_compiler_flags(OUT_VAR)
#
function(vcpkg_assemble_compiler_flags CXXFLAGS_DEBUG CXXFLAGS_RELEASE)
    if (NOT UNIX)
        return()
    endif ()

    set(_calc_FLAGS)
    if (CMAKE_C_FLAGS_INIT)
        set (_calc_FLAGS "${CMAKE_C_FLAGS_INIT}")
    else ()
        set (_calc_FLAGS "-std=c++17")
    endif ()

    if (CMAKE_C_FLAGS_INIT_DEBUG)
        set (_calc_FLAGS_DEBUG "${CMAKE_C_FLAGS_INIT_DEBUG}")
    else ()
        set (_calc_FLAGS_DEBUG "-g")
    endif ()

    if (CMAKE_C_FLAGS_INIT_RELEASE)
        set (_calc_FLAGS_RELEASE "${CMAKE_C_FLAGS_INIT_RELEASE}")
    else ()
        set (_calc_FLAGS_RELEASE "-O3 -DNDEBUG")
    endif ()

    if (CMAKE_CCC_FLAGS_INIT_DEBUG)
        set (_calc_CXXFLAGS_DEBUG "${CMAKE_CXX_FLAGS_INIT_DEBUG}")
    else ()
        set (_calc_CXXFLAGS_DEBUG "-g")
    endif ()

    if (CMAKE_C_FLAGS_INIT_RELEASE)
        set (_calc_CXXFLAGS_RELEASE "${CMAKE_CXX_FLAGS_INIT_RELEASE}")
    else ()
        set (_calc_CXXFLAGS_RELEASE "-O3 -DNDEBUG")
    endif ()

    if (CMAKE_CXX_FLAGS_INIT)
        set (_calc_CXXFLAGS "${CMAKE_CXX_FLAGS_INIT}")
    endif ()

    if (CMAKE_C_VISIBILITY_PRESET)
        set(_calc_FLAGS "${_calc_FLAGS} -fvisibility=${CMAKE_C_VISIBILITY_PRESET}")
    endif ()

    if (CMAKE_CXX_VISIBILITY_PRESET)
        set(_calc_CXXFLAGS "${_calc_CXXFLAGS} -fvisibility=${CMAKE_CXX_VISIBILITY_PRESET}")
    endif ()

    if (CMAKE_VISIBILITY_INLINES_HIDDEN)
        set(_calc_CXXFLAGS "${_calc_CXXFLAGS} -fvisibility-inlines-hidden")
    endif ()

    if (CMAKE_POSITION_INDEPENDENT_CODE)
        set(_calc_FLAGS "${_calc_FLAGS} -fPIC")
        set(_calc_CXXFLAGS "${_calc_CXXFLAGS} -fPIC")
    endif ()

    if (CMAKE_SYSROOT)
        set(_calc_FLAGS "${_calc_FLAGS} --sysroot=${CMAKE_SYSROOT}")
        set(_calc_CXXFLAGS "${_calc_CXXFLAGS} --sysroot=${CMAKE_SYSROOT}")
    endif ()

    set (${CXXFLAGS_DEBUG} "${_calc_FLAGS} ${_calc_CXXFLAGS_DEBUG}" PARENT_SCOPE)
    set (${CXXFLAGS_RELEASE} "${_calc_FLAGS} ${_calc_CXXFLAGS_RELEASE}" PARENT_SCOPE)
endfunction()
