include(vcpkg_common_functions)

set(MAJOR 1)
set(MINOR 67)
set(REVISION 0)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(VERSION_UNDERSCORE ${MAJOR}_${MINOR}_${REVISION})
set(PACKAGE_NAME ${PORT}_${VERSION_UNDERSCORE})
if (CMAKE_HOST_WIN32)
    set(PACKAGE ${PACKAGE_NAME}.7z)
    set(SHA_SUM 5d6d4e93e48a6853ba84ae00dd73e1c6a55734c1de6654a1801b5efb54a0ea36c34fa8877ee08c3be0944a748ba6961bfb37c09e2ad583886e8ec4042e279de0)
else ()
    set(PACKAGE ${PACKAGE_NAME}.tar.bz2)
    set(SHA_SUM 82bf33d7d2c3db109c9d1f12d40bc2d364c8c95262386f906ccd1a71cd71433bcc01829e968b4a13a5003cf0b50cbdf0b435a1d76530cea7bb05725c327411e8)
endif ()

set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src/${PACKAGE_NAME})
set(BUILD_PATH_DEBUG ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-debug)
set(BUILD_PATH_RELEASE ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-release)

vcpkg_download_distfile(ARCHIVE
    URLS "http://dl.bintray.com/boostorg/release/${VERSION}/source/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 ${SHA_SUM}
)

file(REMOVE_RECURSE ${BUILD_PATH_DEBUG})
file(REMOVE_RECURSE ${BUILD_PATH_RELEASE})

vcpkg_extract_source_archive(${ARCHIVE} ${CURRENT_BUILDTREES_DIR}/src)

vcpkg_assemble_compiler_flags(BOOST_CXX_FLAGS_DEBUG BOOST_CXX_FLAGS_RELEASE)
set (BOOST_CXX_FLAGS ${BOOST_CXX_FLAGS_DEBUG})
configure_file(${CMAKE_CURRENT_LIST_DIR}/user-config.jam.in ${BUILD_PATH_DEBUG}/user-config-vcpkg.jam @ONLY)
set (BOOST_CXX_FLAGS ${BOOST_CXX_FLAGS_RELEASE})
configure_file(${CMAKE_CURRENT_LIST_DIR}/user-config.jam.in ${BUILD_PATH_RELEASE}/user-config-vcpkg.jam @ONLY)

if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
    set (ADDRESS_MODEL 64)
else ()
    set (ADDRESS_MODEL 32)
endif ()

if (VCPKG_CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(THREADING single)
else ()
    set(THREADING multi)
endif ()

if (TRIPLET_SYSTEM_ARCH MATCHES "arm")
    set(ARCHITECTURE arm)
else ()
    set(ARCHITECTURE x86)   
endif()

set (B2_OPTIONS
    --debug-configuration
    --ignore-site-config
    --disable-icu
    architecture=${ARCHITECTURE}
    threading=${THREADING}
    address-model=${ADDRESS_MODEL}
)

if (NOT VCPKG_CMAKE_SYSTEM_NAME)
    # windows native build
    set(CONFIG_CMD bootstrap.bat)
    if(VCPKG_PLATFORM_TOOLSET MATCHES "v141")
        set (TOOLSET msvc-14.1)
    elseif(VCPKG_PLATFORM_TOOLSET MATCHES "v140")
        set (TOOLSET msvc-14.0)
    endif ()

    if (STATIC_RUNTIME)
        list(APPEND B2_OPTIONS --layout=tagged runtime-link=static)
    else ()
        list(APPEND B2_OPTIONS --layout=tagged)
    endif ()
else ()
    set(CONFIG_CMD sh ./bootstrap.sh)
    list(APPEND B2_OPTIONS --layout=system)
    set (TOOLSET gcc-vcpkg)
endif ()

if (VCPKG_CMAKE_SYSTEM_NAME STREQUAL Windows AND CMAKE_HOST_UNIX)
    list(APPEND B2_OPTIONS --target-os=windows)
elseif(VCPKG_CMAKE_SYSTEM_NAME STREQUAL Linux AND NOT CMAKE_HOST_UNIX)
    list(APPEND B2_OPTIONS --target-os=linux)
endif ()

list(APPEND B2_OPTIONS --toolset=${TOOLSET})

if(VCPKG_CRT_LINKAGE STREQUAL "dynamic")
    list(APPEND B2_OPTIONS runtime-link=shared)
else()
    list(APPEND B2_OPTIONS runtime-link=static)
endif()

if (VCPKG_LIBRARY_LINKAGE STREQUAL "dynamic")
    list(APPEND B2_OPTIONS link=shared)
else()
    list(APPEND B2_OPTIONS link=static)
endif()

if ("zlib" IN_LIST FEATURES)
    list(APPEND B2_OPTIONS_DBG -s ZLIB_LIBRARY_PATH="${CURRENT_INSTALLED_DIR}/debug/lib")
    list(APPEND B2_OPTIONS_REL -s ZLIB_LIBRARY_PATH="${CURRENT_INSTALLED_DIR}/lib")
    list(APPEND B2_OPTIONS -s ZLIB_INCLUDE="${CURRENT_INSTALLED_DIR}/include")
else ()
    list(APPEND B2_OPTIONS -s NO_ZLIB=1)
endif ()

if ("bzip2" IN_LIST FEATURES)
    list(APPEND B2_OPTIONS_DBG -s BZIP2_LIBRARY_PATH="${CURRENT_INSTALLED_DIR}/debug/lib")
    list(APPEND B2_OPTIONS_REL -s BZIP2_LIBRARY_PATH="${CURRENT_INSTALLED_DIR}/lib")
    list(APPEND B2_OPTIONS -s BZIP2_INCLUDE="${CURRENT_INSTALLED_DIR}/include")
else ()
    list(APPEND B2_OPTIONS -s NO_BZIP2=1)
endif ()

if ("lzma" IN_LIST FEATURES)
    list(APPEND B2_OPTIONS_DBG -s LZMA_LIBRARY_PATH="${CURRENT_INSTALLED_DIR}/debug/lib")
    list(APPEND B2_OPTIONS_REL -s LZMA_LIBRARY_PATH="${CURRENT_INSTALLED_DIR}/lib")
    list(APPEND B2_OPTIONS -s LZMA_INCLUDE="${CURRENT_INSTALLED_DIR}/include")
else ()
    list(APPEND B2_OPTIONS -s NO_LZMA=1)
endif ()

set(WITH_COMPONENTS)
if ("program-options" IN_LIST FEATURES)
    list(APPEND WITH_COMPONENTS --with-program_options)
endif ()

if ("filesystem" IN_LIST FEATURES)
    list(APPEND WITH_COMPONENTS --with-filesystem)
endif ()

if ("system" IN_LIST FEATURES)
    list(APPEND WITH_COMPONENTS --with-system)
endif ()

if ("date-time" IN_LIST FEATURES)
    list(APPEND WITH_COMPONENTS --with-date_time)
endif ()

if ("thread" IN_LIST FEATURES)
    list(APPEND WITH_COMPONENTS --with-thread)
endif ()

if ("iostreams" IN_LIST FEATURES)
    list(APPEND WITH_COMPONENTS --with-iostreams)
endif ()

if ("regex" IN_LIST FEATURES)
    list(APPEND WITH_COMPONENTS --with-regex)
endif ()

if ("math" IN_LIST FEATURES)
    list(APPEND WITH_COMPONENTS --with-math)
endif ()

if (NOT WITH_COMPONENTS)
    # if no components are selected build with system, otherwise all components are built
    list(APPEND B2_OPTIONS --with-system)
else ()
    list(APPEND B2_OPTIONS ${WITH_COMPONENTS})
endif ()

message(STATUS "Configuring ${TARGET_TRIPLET}")
vcpkg_execute_required_process(
    COMMAND ${CONFIG_CMD}
    WORKING_DIRECTORY ${SOURCE_PATH}
    LOGNAME ${PORT}-configure-${TARGET_TRIPLET}
)

message(STATUS "Building ${TARGET_TRIPLET}-dbg")
vcpkg_execute_required_process(
    COMMAND ./b2
    --stagedir=${CURRENT_PACKAGES_DIR}/debug
    --build-dir=${BUILD_PATH_DEBUG}
    --user-config=${BUILD_PATH_DEBUG}/user-config-vcpkg.jam
    "${B2_OPTIONS_DBG}"
    "${B2_OPTIONS}"
    variant=debug
    stage
    WORKING_DIRECTORY ${SOURCE_PATH}
    LOGNAME ${PORT}-build-${TARGET_TRIPLET}-debug
)

message(STATUS "Building ${TARGET_TRIPLET}-rel")
vcpkg_execute_required_process(
    COMMAND ./b2
    --prefix=${CURRENT_PACKAGES_DIR}
    --build-dir=${BUILD_PATH_RELEASE}
    --user-config=${BUILD_PATH_RELEASE}/user-config-vcpkg.jam
    "${B2_OPTIONS_REL}"
    "${B2_OPTIONS}"
    variant=release
    install
    WORKING_DIRECTORY ${SOURCE_PATH}
    LOGNAME ${PORT}-build-${TARGET_TRIPLET}-release
)

if (NOT WITH_COMPONENTS)
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug)
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/lib)
endif ()

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE_1_0.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
