## # vcpkg_install_autotools
##
## Build and install a cmake project.
##
## ## Usage:
## ```cmake
## vcpkg_install_autotools(...)
## ```
##
## ## Parameters:
## See [`vcpkg_build_autotools()`](vcpkg_build_autotools.md).
##
## ## Notes:
## This command transparently forwards to [`vcpkg_build_autotools()`](vcpkg_build_autotools.md), adding a `TARGET install`
## parameter.
##
function(vcpkg_install_autotools)
    vcpkg_build_autotools(LOGFILE_ROOT install TARGET install ${ARGN})
endfunction()
