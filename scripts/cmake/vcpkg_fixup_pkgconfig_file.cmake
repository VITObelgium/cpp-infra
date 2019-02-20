#.rst:
# .. command:: vcpkg_fixup_pkgconfig_file
#
#  Transform all references to the packages dir to the installed dir
#
#  ::
#  vcpkg_fixup_pkgconfig_file()
#
function(vcpkg_fixup_pkgconfig_file)
    if(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "WindowsStore" OR NOT DEFINED VCPKG_CMAKE_SYSTEM_NAME)
        if (NOT MINGW)
            return()
        endif()
    endif()

    cmake_parse_arguments(_vfpf "" "" "NAMES" ${ARGN})

    if(_vfpf_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "vcpkg_fixup_pkgconfig_file was passed extra arguments: ${_vfpf_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT _vfpf_NAMES)
        set(_vfpf_NAMES ${PORT})
    endif ()

    foreach(_vfpf_PKG_CONFIG_NAME IN LISTS _vfpf_NAMES)
        vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/lib/pkgconfig/${_vfpf_PKG_CONFIG_NAME}.pc "${CURRENT_PACKAGES_DIR}" "\${pcfiledir}/../..")
    endforeach()
endfunction()
