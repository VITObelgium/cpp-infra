include(vcpkg_common_functions)

if (EXISTS "${CURRENT_BUILDTREES_DIR}/src/.git")
    file(REMOVE_RECURSE ${CURRENT_BUILDTREES_DIR}/src)
endif()

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO google/googletest
    REF release-1.8.1
    SHA512 e6283c667558e1fd6e49fa96e52af0e415a3c8037afe1d28b7ff1ec4c2ef8f49beb70a9327b7fc77eb4052a58c4ccad8b5260ec90e4bceeac7a46ff59c4369d7
    HEAD_REF master
)

vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}
    PATCHES ${CMAKE_CURRENT_LIST_DIR}/0002-Fix-z7-override.patch
)

string(COMPARE EQUAL "${VCPKG_CRT_LINKAGE}" "dynamic" GTEST_FORCE_SHARED_CRT)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS 
        -DBUILD_GMOCK=ON
        -DBUILD_GTEST=ON
        -Dgtest_force_shared_crt=${GTEST_FORCE_SHARED_CRT}
)

set(ENV{_CL_} "/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")

vcpkg_install_cmake()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(INSTALL ${SOURCE_PATH}/googletest/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/gtest RENAME copyright)

# This block will be unnecessary in googletest 1.9.0 (or later).
# These dll files are installed in ../bin directory by default settings.
if(EXISTS ${CURRENT_PACKAGES_DIR}/lib/gtest.dll)
    file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/bin/)
    file(RENAME ${CURRENT_PACKAGES_DIR}/lib/gtest.dll ${CURRENT_PACKAGES_DIR}/bin/gtest.dll)
    file(RENAME ${CURRENT_PACKAGES_DIR}/lib/gtest_main.dll ${CURRENT_PACKAGES_DIR}/bin/gtest_main.dll)
    file(RENAME ${CURRENT_PACKAGES_DIR}/lib/gmock.dll ${CURRENT_PACKAGES_DIR}/bin/gmock.dll)
    file(RENAME ${CURRENT_PACKAGES_DIR}/lib/gmock_main.dll ${CURRENT_PACKAGES_DIR}/bin/gmock_main.dll)
endif()
if(EXISTS ${CURRENT_PACKAGES_DIR}/debug/lib/gtestd.dll)
    file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/debug/bin/)
    file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/gtestd.dll ${CURRENT_PACKAGES_DIR}/debug/bin/gtestd.dll)
    file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/gtest_maind.dll ${CURRENT_PACKAGES_DIR}/debug/bin/gtest_maind.dll)
    file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/gmockd.dll ${CURRENT_PACKAGES_DIR}/debug/bin/gmockd.dll)
    file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/gmock_maind.dll ${CURRENT_PACKAGES_DIR}/debug/bin/gmock_maind.dll)
endif()

vcpkg_copy_pdbs()
vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/GTest)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/FindGMock.cmake DESTINATION ${CURRENT_PACKAGES_DIR}/share/cmake)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/usage DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT})
