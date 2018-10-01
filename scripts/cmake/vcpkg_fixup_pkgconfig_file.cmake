#.rst:
# .. command:: vcpkg_fixup_pkgconfig_file
#
#  Transform all references to the packages dir to the installed dir
#
#  ::
#  vcpkg_fixup_pkgconfig_file()
#
function(vcpkg_fixup_pkgconfig_file)
    if (NOT (UNIX OR MINGW))
        return()
    endif ()

    cmake_parse_arguments(_vfpf "" "" "NAMES" ${ARGN})

    if(_vfpf_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "vcpkg_fixup_cmake_targets was passed extra arguments: ${_vfpf_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT _vfpf_NAMES)
        vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/lib/pkgconfig/${PORT}.pc "packages/${PORT}_${TARGET_TRIPLET}" "installed/${TARGET_TRIPLET}")
    else()
        foreach(_vfpf_PKG_CONFIG_NAME IN LISTS _vfpf_NAMES)
            vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/lib/pkgconfig/${_vfpf_PKG_CONFIG_NAME}.pc "packages/${PORT}_${TARGET_TRIPLET}" "installed/${TARGET_TRIPLET}")
        endforeach()
    endif()
endfunction()
