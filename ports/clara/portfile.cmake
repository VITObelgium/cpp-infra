include(vcpkg_common_functions)
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO catchorg/Clara
    REF v1.1.4
    SHA512 1ffaaa260175f2643b4a019f1294fc59113961881f22ca3750fd4d90a93d2da01a1e566a79c1faef7848450197272b61fb08e31f444d3083bc332424839248fd
    HEAD_REF master
)

file(INSTALL ${SOURCE_PATH}/single_include/clara.hpp DESTINATION ${CURRENT_PACKAGES_DIR}/include)
file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/clara RENAME copyright)
