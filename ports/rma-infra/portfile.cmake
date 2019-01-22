include(vcpkg_common_functions)
vcpkg_from_git(
    URL https://git.vito.be/scm/marvin-geodynamix/infra.git
    OUT_SOURCE_PATH SOURCE_PATH
    REF 0.9.1
    HEAD_REF master
    SHA512 11e13824b8110a9154f168a64163aa9e7e1e9866922413dc0fb5a7be858297054ae00790374e49fe1ebe0b7bac26030c527ebe64f2779d840ab680cd97b40ac9
)

TEST_FEATURE("gdal" ENABLE_GDAL)
TEST_FEATURE("xml" ENABLE_XML)
TEST_FEATURE("log" ENABLE_LOGGING)
TEST_FEATURE("numeric" ENABLE_NUMERIC)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DINFRA_LOGGING=${ENABLE_LOGGING}
        -DINFRA_GDAL=${ENABLE_GDAL}
        -DINFRA_EMBED_GDAL_DATA=${ENABLE_GDAL}
        -DINFRA_LICENSE_MANAGER=OFF
        -DINFRA_XML=${ENABLE_XML}
        -DINFRA_NUMERIC=${ENABLE_NUMERIC}
        -DINFRA_ENABLE_TESTS=OFF
        -DINFRA_ENABLE_BENCHMARKS=OFF
        -DINFRA_ENABLE_DOCUMENTATION=OFF
        -DINFRA_UI_COMPONENTS=OFF
)

vcpkg_install_cmake()
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)

vcpkg_fixup_cmake_targets(CONFIG_PATH "lib/cmake/infra")
vcpkg_copy_pdbs()
file(WRITE ${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright "Copyright VITO NV\n")
