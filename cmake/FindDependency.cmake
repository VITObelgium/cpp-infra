include(CMakeParseArguments)

function(find_dependency)
    set (oneValueArgs NAME)
    set (multiValueArgs LIBNAMES DEBUGLIBNAMES)
    cmake_parse_arguments(find_dependency "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    find_library(${find_dependency_NAME}_LIBRARY NAMES ${find_dependency_LIBNAMES})
    find_library(${find_dependency_NAME}_LIBRARY_DEBUG NAMES ${find_dependency_DEBUGLIBNAMES})

    if (${find_dependency_NAME}_LIBRARY AND ${find_dependency_NAME}_LIBRARY_DEBUG)
        set(${find_dependency_NAME}_LIBRARY optimized ${find_dependency_NAME}_LIBRARY debug ${find_dependency_NAME}_LIBRARY_DEBUG)
    endif ()
endfunction()
