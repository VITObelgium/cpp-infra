include(vcpkg_common_functions)
vcpkg_from_git(
    GIT_URL https://git.vito.be/scm/marvin-geodynamix/infra.git
    OUT_SOURCE_PATH SOURCE_PATH
    REF 0.9.1
    HEAD_REF master
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
