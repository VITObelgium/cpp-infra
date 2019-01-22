include(vcpkg_common_functions)

set(MAJOR 1)
set(MINOR 69)
set(REVISION 0)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(VERSION_UNDERSCORE ${MAJOR}_${MINOR}_${REVISION})
set(PACKAGE_NAME ${PORT}_${VERSION_UNDERSCORE})
if (CMAKE_HOST_WIN32)
    set(PACKAGE ${PACKAGE_NAME}.7z)
    set(SHA_SUM 0dcca688f94b6d0fba4b530ed33ea7018487e9ca5a61cf4502784c64885fc42a9fc79cf187b02dc97e6003761bd2ba3747f119517e51338ea091eeda168e317b)
else ()
    set(PACKAGE ${PACKAGE_NAME}.tar.bz2)
    set(SHA_SUM d0e9bb858c44880d56c0291afef6a1b011a62f659a2d8f58dcb6147ea0899f9157bd8db3097896618fee0116847ebeac78b6d0f0fec8a92c3469500828bbe552)
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
    -d2
    --debug-configuration
    --ignore-site-config
    --disable-icu
    architecture=${ARCHITECTURE}
    threading=${THREADING}
    address-model=${ADDRESS_MODEL}
)

set (B2_OPTIONS_DBG
    --stagedir=${CURRENT_PACKAGES_DIR}/debug
    --build-dir=${BUILD_PATH_DEBUG}
    --user-config=${BUILD_PATH_DEBUG}/user-config-vcpkg.jam
    variant=debug
)

set (B2_OPTIONS_REL
    --prefix=${CURRENT_PACKAGES_DIR}
    --build-dir=${BUILD_PATH_RELEASE}
    --user-config=${BUILD_PATH_RELEASE}/user-config-vcpkg.jam
    variant=release
)

if (NOT VCPKG_CMAKE_SYSTEM_NAME)
    # windows native build
    set(CONFIG_CMD bootstrap.bat)
    set(BUILD_CMD b2)
    if(VCPKG_PLATFORM_TOOLSET MATCHES "v141")
        set (TOOLSET msvc-14.1)
    elseif(VCPKG_PLATFORM_TOOLSET MATCHES "v140")
        set (TOOLSET msvc-14.0)
    endif ()

    if (STATIC_RUNTIME)
        list(APPEND B2_OPTIONS --layout=versioned runtime-link=static)
    else ()
        list(APPEND B2_OPTIONS --layout=versioned)
    endif ()
else ()
    set(CONFIG_CMD sh ./bootstrap.sh)
    set(BUILD_CMD ./b2)
    list(APPEND B2_OPTIONS --layout=system)
    if(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set (TOOLSET darwin-vcpkg)
    else()
        set (TOOLSET gcc-vcpkg)
    endif()
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
    list(APPEND B2_OPTIONS_DBG -s ZLIB_LIBRARY_PATH=${CURRENT_INSTALLED_DIR}/debug/lib)
    list(APPEND B2_OPTIONS_REL -s ZLIB_LIBRARY_PATH=${CURRENT_INSTALLED_DIR}/lib)
    if (NOT VCPKG_CMAKE_SYSTEM_NAME)
        list(APPEND B2_OPTIONS_DBG -s ZLIB_NAME=zlibd)
        list(APPEND B2_OPTIONS_REL -s ZLIB_NAME=zlib)
    endif ()
    list(APPEND B2_OPTIONS -s ZLIB_INCLUDE=${CURRENT_INSTALLED_DIR}/include)
else ()
    list(APPEND B2_OPTIONS -s NO_ZLIB=1)
endif ()

if ("bzip2" IN_LIST FEATURES)
    list(APPEND B2_OPTIONS_DBG -s BZIP2_LIBRARY_PATH=${CURRENT_INSTALLED_DIR}/debug/lib)
    list(APPEND B2_OPTIONS_REL -s BZIP2_LIBRARY_PATH=${CURRENT_INSTALLED_DIR}/lib)
    list(APPEND B2_OPTIONS -s BZIP2_INCLUDE=${CURRENT_INSTALLED_DIR}/include)
else ()
    list(APPEND B2_OPTIONS -s NO_BZIP2=1)
endif ()

if ("lzma" IN_LIST FEATURES)
    list(APPEND B2_OPTIONS_DBG -s LZMA_LIBRARY_PATH=${CURRENT_INSTALLED_DIR}/debug/lib)
    list(APPEND B2_OPTIONS_REL -s LZMA_LIBRARY_PATH=${CURRENT_INSTALLED_DIR}/lib)
    list(APPEND B2_OPTIONS -s LZMA_INCLUDE=${CURRENT_INSTALLED_DIR}/include)
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

if ("timer" IN_LIST FEATURES)
    list(APPEND WITH_COMPONENTS --with-timer)
endif ()

if ("python" IN_LIST FEATURES)
    set(Python3_VERSION_MAJOR 3)
    set(Python3_VERSION_MINOR 7)
    set(Python3_EXECUTABLE /projects/urbflood/.miniconda3/bin/python)
    set(Python3_INCLUDE_DIRS /projects/urbflood/.miniconda3/include/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}m)
    set(Python3_LIBRARIES /projects/urbflood/.miniconda3/lib/libpython${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}.so)
    list(APPEND WITH_COMPONENTS --with-python)
endif ()

if (NOT WITH_COMPONENTS)
    # if no components are selected build with system, otherwise all components are built
    list(APPEND B2_OPTIONS --with-system)
else ()
    list(APPEND B2_OPTIONS ${WITH_COMPONENTS})
endif ()

vcpkg_assemble_compiler_cxxflags(DEBUG BOOST_CXX_FLAGS_DEBUG RELEASE BOOST_CXX_FLAGS_RELEASE)
set (BOOST_CXX_FLAGS ${BOOST_CXX_FLAGS_DEBUG})
configure_file(${CMAKE_CURRENT_LIST_DIR}/user-config.jam.in ${BUILD_PATH_DEBUG}/user-config-vcpkg.jam @ONLY)
set (BOOST_CXX_FLAGS ${BOOST_CXX_FLAGS_RELEASE})
configure_file(${CMAKE_CURRENT_LIST_DIR}/user-config.jam.in ${BUILD_PATH_RELEASE}/user-config-vcpkg.jam @ONLY)

message(STATUS "Configuring ${TARGET_TRIPLET}")
vcpkg_execute_required_process(
    COMMAND ${CONFIG_CMD}
    WORKING_DIRECTORY ${SOURCE_PATH}
    LOGNAME ${PORT}-configure-${TARGET_TRIPLET}
)

message(STATUS "Building ${TARGET_TRIPLET}-dbg")
if (VCPKG_VERBOSE)
    message(STATUS "COMMAND ${BUILD_CMD} ${B2_OPTIONS_DBG} ${B2_OPTIONS} stage")
endif ()
vcpkg_execute_required_process(
    COMMAND ${BUILD_CMD} ${B2_OPTIONS_DBG} ${B2_OPTIONS} stage
    WORKING_DIRECTORY ${SOURCE_PATH}
    LOGNAME ${PORT}-build-${TARGET_TRIPLET}-debug
)

message(STATUS "Building ${TARGET_TRIPLET}-rel")
vcpkg_execute_required_process(
    COMMAND ${BUILD_CMD} ${B2_OPTIONS_REL} ${B2_OPTIONS} install
    WORKING_DIRECTORY ${SOURCE_PATH}
    LOGNAME ${PORT}-build-${TARGET_TRIPLET}-release
)

if (NOT WITH_COMPONENTS)
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug)
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/lib)
endif ()

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE_1_0.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
