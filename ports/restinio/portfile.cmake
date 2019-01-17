include(vcpkg_common_functions)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 4)
set(VERSION_REVISION 8)

set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION})
set(PACKAGE ${PORT}-${VERSION}-full.tar.bz2)

set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/${PORT}-${VERSION}-full)
vcpkg_download_distfile(ARCHIVE
    URLS "https://bitbucket.org/sobjectizerteam/${PORT}-${VERSION_MAJOR}.${VERSION_MINOR}/downloads/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 b77f1ef3c769a02ad53c777f04a4df3fa027ffa7ac8f09aca6e813c16de3b4bf69c5e6161c5960918c0ce9752f8f8b9f8f2a3a0b5c744bad969f6186583bd70a
)

vcpkg_extract_source_archive(${ARCHIVE})
vcpkg_replace_string(${SOURCE_PATH}/dev/CMakeLists.txt "add_subdirectory(so_5)" "")
vcpkg_replace_string(${SOURCE_PATH}/dev/restinio/CMakeLists.txt "fmt::fmt-header-only" "fmt::fmt")

if("boost" IN_LIST FEATURES)
    set(ASIO_OPT -DRESTINIO_USE_BOOST_ASIO=static)
elseif("asio" IN_LIST FEATURES)
    set(ASIO_OPT -DRESTINIO_USE_BOOST_ASIO=none)
else()
    message(FATAL_ERROR "boost or asio must be active as a feature")
endif()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}/dev
    PREFER_NINJA
    OPTIONS
        -DRESTINIO_INSTALL=ON
        -DRESTINIO_TEST=OFF
        -DRESTINIO_SAMPLE=OFF
        -DRESTINIO_BENCH=OFF
        -DRESTINIO_FIND_DEPS=ON
        ${ASIO_OPT}
)

vcpkg_install_cmake()

vcpkg_fixup_cmake_targets(CONFIG_PATH "lib/cmake/restinio")

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/lib ${CURRENT_PACKAGES_DIR}/debug)

# Handle copyright
file(COPY ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/restinio/copyright)

