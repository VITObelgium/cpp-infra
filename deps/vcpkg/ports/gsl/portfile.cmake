#header-only library
include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Microsoft/GSL
    REF v2.0.0
    SHA512 7339527222c8a97a94c0bb4038b3d142045ec5d80995e628574ac96f4d9d13c41ad70fbe0d8390586dc0db8d9ea55107dbc95de80f7335eb78ef9d2e7047d726
    HEAD_REF master
)

file(INSTALL ${SOURCE_PATH}/include/ DESTINATION ${CURRENT_PACKAGES_DIR}/include)
file(INSTALL ${CMAKE_CURRENT_LIST_DIR}/FindGsl.cmake DESTINATION ${CURRENT_PACKAGES_DIR}/share/cmake)
# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
