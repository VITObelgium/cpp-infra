function(vcpkg_install_meson)

    vcpkg_find_acquire_program(NINJA)

    unset(ENV{DESTDIR}) # installation directory was already specified with '--prefix' option

    if(NOT DEFINED VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL "release")
        message(STATUS "Package ${TARGET_TRIPLET}-rel")
        vcpkg_execute_required_process(
            COMMAND ${NINJA} install -v
            WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel
            LOGNAME package-${TARGET_TRIPLET}-rel
        )
    endif()

    if(NOT DEFINED VCPKG_BUILD_TYPE OR VCPKG_BUILD_TYPE STREQUAL "debug")
        message(STATUS "Package ${TARGET_TRIPLET}-dbg")
        vcpkg_execute_required_process(
            COMMAND ${NINJA} install -v
            WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg
            LOGNAME package-${TARGET_TRIPLET}-dbg
        )
    endif()

endfunction()
