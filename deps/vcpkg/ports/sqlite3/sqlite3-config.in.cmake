
if("@VCPKG_LIBRARY_LINKAGE@" STREQUAL "static")
    include(CMakeFindDependencyMacro)
    find_dependency(Threads)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/sqlite3-targets.cmake)
