#.rst:
# .. command:: vcpkg_assemble_compiler_cflags
# .. command:: vcpkg_assemble_compiler_cxxflags
#
#  Create compiler flags based on the current cmake toolchain
#  to pass to other build systems
#
#  ::
#  vcpkg_assemble_compiler_cflags(OUT_VAR_DEBUG OUT_VAR_RELEASE)
#  vcpkg_assemble_compiler_cxxflags(OUT_VAR_DEBUG OUT_VAR_RELEASE)
#
function(vcpkg_assemble_compiler_cflags)
    if (NOT UNIX)
        return()
    endif ()

    cmake_parse_arguments(_cf "" "DEBUG;RELEASE" "" ${ARGN})

    set(_calc_CFLAGS)
    if (CMAKE_C_FLAGS_INIT)
        set (_calc_CFLAGS "${CMAKE_C_FLAGS_INIT}")
    endif ()

    if (CMAKE_C_FLAGS_INIT_DEBUG)
        set (_calc_CFLAGS_DEBUG "${CMAKE_C_FLAGS_INIT_DEBUG}")
    else ()
        set (_calc_CFLAGS_DEBUG "-g")
    endif ()

    if (CMAKE_C_FLAGS_INIT_RELEASE)
        set (_calc_CFLAGS_RELEASE "${CMAKE_C_FLAGS_INIT_RELEASE}")
    else ()
        set (_calc_CFLAGS_RELEASE "-O3 -DNDEBUG")
    endif ()

    if (CMAKE_C_VISIBILITY_PRESET)
        set(_calc_CFLAGS "${_calc_CFLAGS} -fvisibility=${CMAKE_C_VISIBILITY_PRESET}")
    endif ()

    if (CMAKE_POSITION_INDEPENDENT_CODE)
        set(_calc_CFLAGS "${_calc_CFLAGS} -fPIC")
    endif ()

    if (CMAKE_SYSROOT)
        set(_calc_CFLAGS "${_calc_CFLAGS} --sysroot=${CMAKE_SYSROOT}")
    endif ()

    if (CMAKE_OSX_DEPLOYMENT_TARGET)
        set(_calc_CFLAGS "${_calc_CFLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif ()

    set (${_cf_DEBUG} "${_calc_CFLAGS} ${_calc_CFLAGS_DEBUG} ${VCPKG_C_FLAGS} ${VCPKG_C_FLAGS_DEBUG}" PARENT_SCOPE)
    set (${_cf_RELEASE} "${_calc_CFLAGS} ${_calc_CFLAGS_RELEASE} ${VCPKG_C_FLAGS} ${VCPKG_C_FLAGS_RELEASE}" PARENT_SCOPE)
endfunction()

function(vcpkg_assemble_compiler_cxxflags)
    if (NOT UNIX)
        return()
    endif ()

    cmake_parse_arguments(_cf "" "STANDARD;DEBUG;RELEASE" "" ${ARGN})

    set(_calc_CXXFLAGS)
    if (CMAKE_CXX_FLAGS_INIT)
        set (_calc_CXXFLAGS "${CMAKE_CXX_FLAGS_INIT}")
    elseif (_cf_STANDARD)
        set (_calc_CXXFLAGS "-std=c++${_cf_STANDARD}")
    else ()
        set (_calc_CXXFLAGS "-std=c++17")
    endif ()

    if (CMAKE_CXX_FLAGS_INIT_DEBUG)
        set (_calc_CXXFLAGS_DEBUG "${CMAKE_CXX_FLAGS_INIT_DEBUG}")
    else ()
        set (_calc_CXXFLAGS_DEBUG "-g")
    endif ()

    if (CMAKE_CXX_FLAGS_INIT_RELEASE)
        set (_calc_CXXFLAGS_RELEASE "${CMAKE_CXX_FLAGS_INIT_RELEASE}")
    else ()
        set (_calc_CXXFLAGS_RELEASE "-O3 -DNDEBUG")
    endif ()

    if (CMAKE_CXX_FLAGS_INIT_DEBUG)
        set (_calc_CXXFLAGS_DEBUG "${CMAKE_CXX_FLAGS_INIT_DEBUG}")
    else ()
        set (_calc_CXXFLAGS_DEBUG "-g")
    endif ()

    if (CMAKE_CXX_FLAGS_INIT_RELEASE)
        set (_calc_CXXFLAGS_RELEASE "${CMAKE_CXX_FLAGS_INIT_RELEASE}")
    else ()
        set (_calc_CXXFLAGS_RELEASE "-O3 -DNDEBUG")
    endif ()

    if (CMAKE_CXX_FLAGS_INIT)
        set (_calc_CXXFLAGS "${CMAKE_CXX_FLAGS_INIT}")
    endif ()

    if (CMAKE_CXX_VISIBILITY_PRESET)
        set(_calc_CXXFLAGS "${_calc_CXXFLAGS} -fvisibility=${CMAKE_CXX_VISIBILITY_PRESET}")
    endif ()

    if (CMAKE_VISIBILITY_INLINES_HIDDEN)
        set(_calc_CXXFLAGS "${_calc_CXXFLAGS} -fvisibility-inlines-hidden")
    endif ()

    if (CMAKE_POSITION_INDEPENDENT_CODE)
        set(_calc_CXXFLAGS "${_calc_CXXFLAGS} -fPIC")
    endif ()

    if (CMAKE_SYSROOT)
        set(_calc_CXXFLAGS "${_calc_CXXFLAGS} --sysroot=${CMAKE_SYSROOT}")
    endif ()

    if (CMAKE_OSX_DEPLOYMENT_TARGET)
        set(_calc_CXXFLAGS "${_calc_CXXFLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif ()

    set (${_cf_DEBUG} "${_calc_CXXFLAGS} ${_calc_CXXFLAGS_DEBUG} ${VCPKG_CXX_FLAGS} ${VCPKG_CXX_FLAGS_DEBUG}" PARENT_SCOPE)
    set (${_cf_RELEASE} "${_calc_CXXFLAGS} ${_calc_CXXFLAGS_RELEASE} ${VCPKG_CXX_FLAGS} ${VCPKG_CXX_FLAGS_RELEASE}" PARENT_SCOPE)
endfunction()

function(vcpkg_assemble_linker_cflags)
    if (NOT UNIX)
        return()
    endif ()

    cmake_parse_arguments(_cf "" "DEBUG;RELEASE" "" ${ARGN})
    
    set (${_lf_DEBUG} "${_calc_LDFLAGS} ${_calc_LDFLAGS_DEBUG} ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${VCPKG_LINKER_FLAGS} ${VCPKG_LINKER_FLAGS_DEBUG}" PARENT_SCOPE)
    set (${_lf_RELEASE} "${_calc_LDFLAGS} ${_calc_LDFLAGS_RELEASE} ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${VCPKG_LINKER_FLAGS} ${VCPKG_LINKER_FLAGS_RELEASE}" PARENT_SCOPE)
endfunction()
