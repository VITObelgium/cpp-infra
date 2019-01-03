include(vcpkg_common_functions)

set(MAJOR 5)
set(MINOR 12)
set(REVISION 0)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(RELEASE official)
if (WIN32)
    set(TARBALL_EXTENSION zip)
    set (SHASUM c954418c6391955f8ff15a6e1b0ad2d08d75f21399213bd019dc46a714c7e5a58d5c094d8ff96d5deb47e4d81ffe5348960c55818237037e49cbad36d03feed8)
else ()
    set(TARBALL_EXTENSION tar.xz)
    set (SHASUM 0dd03d2645fb6dac5b58c8caf92b4a0a6900131f1ccfb02443a0df4702b5da0458f4c45e758d1b929ec709b0f4b36900df2fd60a058af9cc8c1a0748b6d57aae)
endif ()
set(PACKAGE_NAME qt-everywhere-src-${VERSION})
set(PACKAGE ${PACKAGE_NAME}.${TARBALL_EXTENSION})

find_package(PythonInterp REQUIRED)
get_filename_component(PYTHON_DIR ${PYTHON_EXECUTABLE} DIRECTORY)
message(STATUS "Python directory: ${PYTHON_DIR}")
vcpkg_add_to_path(${PYTHON_DIR})

if (CMAKE_HOST_WIN32 AND NOT MINGW)
    vcpkg_find_acquire_program("JOM")
endif ()

# Extract source into architecture specific directory, because GDALs' build currently does not
# support out of source builds.
set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src-${TARGET_TRIPLET})
message(STATUS "Qt source path ${SOURCE_PATH}")
vcpkg_download_distfile(ARCHIVE
    URLS "http://download.qt.io/archive/qt/${MAJOR}.${MINOR}/${VERSION}/single/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 ${SHASUM}
)

vcpkg_extract_source_archive(${ARCHIVE} ${SOURCE_PATH})
vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}/${PACKAGE_NAME}
    PATCHES
    ${CMAKE_CURRENT_LIST_DIR}/support-static-builds-with-cmake.patch
    ${CMAKE_CURRENT_LIST_DIR}/mingw-cmake-prl-file-has-no-lib-prefix.patch
 )

 set(OSX_LEGACY_SDK OFF)

 # z lib name is zlib.lib on windows, not zdll.lib
 vcpkg_replace_string(${SOURCE_PATH}/${PACKAGE_NAME}/qtbase/configure.json "-lzdll" "-lzlib")
 # jpeg lib name is jpeg.lib on windows, not libjpeg.lib
 vcpkg_replace_string(${SOURCE_PATH}/${PACKAGE_NAME}/qtbase/src/gui/configure.json "-llibjpeg" "-ljpeg")

 # copy the g++-cluster compiler specification
file(COPY ${CMAKE_CURRENT_LIST_DIR}/linux-g++-cluster DESTINATION ${SOURCE_PATH}/${PACKAGE_NAME}/qtbase/mkspecs)

set(OPTIONAL_ARGS)
if(NOT "tools" IN_LIST FEATURES)
    list(APPEND OPTIONAL_ARGS -skip qttools)
endif()

if(NOT "qml" IN_LIST FEATURES)
    list(APPEND OPTIONAL_ARGS -skip qtquickcontrols -skip qtquickcontrols2 -skip qtdeclarative)
endif()

if(NOT "location" IN_LIST FEATURES)
    list(APPEND OPTIONAL_ARGS -skip qtlocation)
endif()

if(NOT "sql" IN_LIST FEATURES)
    list(APPEND OPTIONAL_ARGS -no-feature-sql -no-sql-db2 -no-sql-ibase -no-sql-mysql -no-sql-oci -no-sql-odbc -no-sql-psql -no-sql-sqlite2 -no-sql-sqlite -no-sql-tds)
else ()
    list(APPEND OPTIONAL_ARGS -sql-sqlite -system-sqlite)
endif()

set(QT_OPTIONS
    -I ${CURRENT_INSTALLED_DIR}/include
    -verbose
    -opensource
    -confirm-license
    -nomake examples
    -nomake tests
    -no-dbus
    -no-icu
    -no-openssl
    -no-glib
    -system-zlib
    -no-cups
    -system-freetype
    -system-harfbuzz
    -system-libpng
    -system-libjpeg
    -system-pcre
    -skip qtdoc
    -skip qt3d
    -skip qtactiveqt
    -skip qtcanvas3d
    -skip qtconnectivity
    -skip qtdatavis3d
    -skip qtgamepad
    -skip qtmultimedia
    -skip qtpurchasing
    -skip qtremoteobjects
    -skip qtscript
    -skip qtsensors
    -skip qtserialbus
    -skip qtserialport
    -skip qtscxml
    -skip qtspeech
    -skip qtvirtualkeyboard
    -skip qtwebengine
    -skip qtwebchannel
    -skip qtwebsockets
    -skip qtwebview
    -skip qtwinextras
    -no-feature-testlib
    ${OPTIONAL_ARGS}
)

if (VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    list(APPEND QT_OPTIONS -static)
else ()
    list(APPEND QT_OPTIONS -shared)
endif ()

if(VCPKG_CRT_LINKAGE STREQUAL "static")
    list(APPEND QT_OPTIONS -static-runtime)
endif()

if (CMAKE_CXX_VISIBILITY_PRESET STREQUAL hidden)
    list(APPEND QT_OPTIONS -reduce-exports)
else ()
    list(APPEND QT_OPTIONS -no-reduce-exports)
endif ()

if (CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    list(APPEND QT_OPTIONS -ltcg)
endif ()

include(ProcessorCount)
ProcessorCount(NUM_CORES)
set(BUILD_COMMAND make -j${NUM_CORES})
set(EXEC_COMMAND)

set(PLATFORM_OPTIONS)
if(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "WindowsStore" OR NOT DEFINED VCPKG_CMAKE_SYSTEM_NAME)
    find_program(JOM jom)
    if (JOM)
        set(BUILD_COMMAND ${JOM})
    else ()
        find_program(NMAKE nmake REQUIRED)
        set(BUILD_COMMAND ${NMAKE})
    endif ()

    set(CONFIG_SUFFIX .bat)
    list(APPEND QT_OPTIONS -platform win32-msvc2017)
    list(APPEND PLATFORM_OPTIONS -opengl desktop -mp)
elseif(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (HOST MATCHES "x86_64-unknown-linux-gnu")
        set(PLATFORM -platform linux-g++-cluster)
        list(APPEND PLATFORM_OPTIONS -no-opengl -sysroot ${CMAKE_SYSROOT})
    elseif (CMAKE_COMPILER_IS_GNUCXX)
        set(PLATFORM -platform linux-g++)
    else ()
        set(PLATFORM -platform linux-clang)
    endif ()
    list(APPEND PLATFORM_OPTIONS -no-pch -c++std c++1z)
    #-device-option CROSS_COMPILE=${CROSS}
elseif(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    if (OSX_LEGACY_SDK)
        set(OSX_SDK_VERSION 10.13)
        list(APPEND PLATFORM_OPTIONS -c++std c++14)
    else ()
        set(OSX_SDK_VERSION 10.14)
        list(APPEND PLATFORM_OPTIONS -c++std c++1z)
    endif ()

    list(APPEND PLATFORM_OPTIONS -no-pch -sdk macosx${OSX_SDK_VERSION})

    vcpkg_replace_string(${SOURCE_PATH}/${PACKAGE_NAME}/qtbase/mkspecs/macx-clang/qmake.conf
        "QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.11"
        "QMAKE_MACOSX_DEPLOYMENT_TARGET = ${OSX_SDK_VERSION}"
    )
elseif (MINGW AND (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin"))
    set(PLATFORM -xplatform win32-g++)
    list(APPEND PLATFORM_OPTIONS -opengl desktop)
elseif (MINGW)
    set(PLATFORM -platform win32-g++)
    list(APPEND PLATFORM_OPTIONS -opengl desktop)
    set(EXEC_COMMAND sh)
endif()

list(APPEND QT_OPTIONS ${PLATFORM} ${PLATFORM_OPTIONS})

set(QT_OPTIONS_REL
    ${QT_OPTIONS}
    -release
    -L ${CURRENT_INSTALLED_DIR}/lib
    -prefix ${CURRENT_INSTALLED_DIR}
    -extprefix ${CURRENT_PACKAGES_DIR}
    -hostbindir ${CURRENT_PACKAGES_DIR}/tools
    -archdatadir ${CURRENT_PACKAGES_DIR}/share/qt5
    -datadir ${CURRENT_PACKAGES_DIR}/share/qt5
    -plugindir ${CURRENT_PACKAGES_DIR}/share/qt5/plugins
    -qmldir ${CURRENT_PACKAGES_DIR}/qml
)

set(QT_OPTIONS_DBG
    ${QT_OPTIONS}
    -debug
    -L ${CURRENT_INSTALLED_DIR}/lib
    -prefix ${CURRENT_INSTALLED_DIR}/debug
    -extprefix ${CURRENT_PACKAGES_DIR}/debug
    -hostbindir ${CURRENT_INSTALLED_DIR}/debug/tools
    -archdatadir ${CURRENT_INSTALLED_DIR}/debug/share/qt5
    -datadir ${CURRENT_INSTALLED_DIR}/debug/share/qt5
    -plugindir ${CURRENT_INSTALLED_DIR}/plugins
    -qmldir ${CURRENT_PACKAGES_DIR}/qml
    -headerdir ${CURRENT_PACKAGES_DIR}/debug/include
)

file(REMOVE_RECURSE ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg)

set(ENV{PKG_CONFIG_LIBDIR} "${CURRENT_INSTALLED_DIR}/lib/pkgconfig")
message(STATUS "Configuring ${TARGET_TRIPLET}-dbg")
if(VCPKG_VERBOSE)
    STRING(JOIN " " QT_ARGS ${QT_OPTIONS_DBG})
    message(STATUS "${SOURCE_PATH}/${PACKAGE_NAME}/configure${CONFIG_SUFFIX} ${QT_ARGS}")
endif()
file(MAKE_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg)
vcpkg_execute_required_process(
    COMMAND ${EXEC_COMMAND} ${SOURCE_PATH}/${PACKAGE_NAME}/configure${CONFIG_SUFFIX} ${QT_OPTIONS_DBG}
    WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg
    LOGNAME qt-configure-${TARGET_TRIPLET}-debug
)

message(STATUS "Configuring ${TARGET_TRIPLET}-rel")
message(STATUS "${QT_OPTIONS_REL}")
file(MAKE_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel)
vcpkg_execute_required_process(
    COMMAND ${EXEC_COMMAND} ${SOURCE_PATH}/${PACKAGE_NAME}/configure${CONFIG_SUFFIX} ${QT_OPTIONS_REL}
    WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel
    LOGNAME qt-configure-${TARGET_TRIPLET}-release
)

message(STATUS "Building ${TARGET_TRIPLET}-dbg")
vcpkg_execute_required_process(
    COMMAND ${BUILD_COMMAND} install
    WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg
    LOGNAME qt-build-${TARGET_TRIPLET}-debug
)

message(STATUS "Building ${TARGET_TRIPLET}-rel")
vcpkg_execute_required_process(
    COMMAND ${BUILD_COMMAND} install
    WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel
    LOGNAME qt-build-${TARGET_TRIPLET}-release
)

# Fix the cmake files
file(RENAME ${CURRENT_PACKAGES_DIR}/lib/cmake ${CURRENT_PACKAGES_DIR}/share/cmake)
vcpkg_execute_required_process(
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/fixcmake.py
    WORKING_DIRECTORY ${CURRENT_PACKAGES_DIR}/share/cmake
    LOGNAME fix-cmake
)

file(GLOB BIN_FILES ${CURRENT_PACKAGES_DIR}/bin/*)
file(COPY ${BIN_FILES} DESTINATION ${CURRENT_PACKAGES_DIR}/tools)
file(REMOVE_RECURSE
    ${CURRENT_PACKAGES_DIR}/debug/bin
    ${CURRENT_PACKAGES_DIR}/debug/tools
    ${CURRENT_PACKAGES_DIR}/debug/include
    ${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig
    ${CURRENT_PACKAGES_DIR}/debug/lib/cmake
    ${CURRENT_PACKAGES_DIR}/debug/share
    ${CURRENT_PACKAGES_DIR}/bin
)

# bootstrap libs are only used for the tools and cause errors on windows as they link to a different crt
file(REMOVE
    ${CURRENT_PACKAGES_DIR}/debug/lib/Qt5Bootstrap.lib
    ${CURRENT_PACKAGES_DIR}/lib/Qt5Bootstrap.lib
)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/${PACKAGE_NAME}/LICENSE.LGPLv3 DESTINATION  ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)