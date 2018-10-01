include(vcpkg_common_functions)

set(MAJOR 5)
set(MINOR 11)
set(REVISION 2)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(RELEASE official)
if (WIN32)
    set(TARBALL_EXTENSION zip)
    set (SHASUM 57865cae59dae95cdcc7bc2b473946b7c23e22cc1aed375207a7b732d742ea741e8d8fff23b14b950e1800cfa1318a4610ec32314f44bbf5c9bc6d3148305545)
else ()
    set(TARBALL_EXTENSION tar.xz)
    set (SHASUM b9bce31322e26e99b4eeaa6bfa78f51480829284289a0a8acffd9d9028050293d20727b3dcd9adc70eddd18c82724e10154462efc6218062083333349a87a7a8)
endif ()
set(PACKAGE_NAME qt-everywhere-src-${VERSION})
set(PACKAGE ${PACKAGE_NAME}.${TARBALL_EXTENSION})

find_package(PythonInterp REQUIRED)

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
message(STATUS "PATCH PATH ${SOURCE_PATH}/${PACKAGE_NAME}")
vcpkg_apply_patches(
    SOURCE_PATH ${SOURCE_PATH}/${PACKAGE_NAME}
    PATCHES
    ${CMAKE_CURRENT_LIST_DIR}/support-static-builds-with-cmake.patch
    ${CMAKE_CURRENT_LIST_DIR}/mingw-cmake-prl-file-has-no-lib-prefix.patch
 )

 # z lib name is zlib.lib on windows, not zdll.lib
 vcpkg_replace_string(${SOURCE_PATH}/${PACKAGE_NAME}/qtbase/configure.json "-lzdll" "-lzlib")
 # jpeg lib name is jpeg.lib on windows, not libjpeg.lib
 vcpkg_replace_string(${SOURCE_PATH}/${PACKAGE_NAME}/qtbase/src/gui/configure.json "-llibjpeg" "-ljpeg")

set(QT_OPTIONS
    -I ${CURRENT_INSTALLED_DIR}/include
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
    -sql-sqlite
    -system-sqlite
    -skip qt3d
    -skip qtactiveqt
    -skip qtcanvas3d
    -skip qtconnectivity
    -skip qtdatavis3d
    -skip qtgamepad
    -skip qtmultimedia
    -skip qtpurchasing
    -skip qtscript
    -skip qtsensors
    -skip qtserialbus
    -skip qtserialport
    -skip qtscxml
    -skip qtwebengine
    -skip qtwebchannel
    -skip qtwebsockets
    -skip qtwebview
    -skip qtwinextras
)

if (${VCPKG_LIBRARY_LINKAGE} STREQUAL "static")
    list(APPEND QT_OPTIONS -static)
else ()
    list(APPEND QT_OPTIONS -shared)
endif ()

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
    find_program(NMAKE nmake REQUIRED)
    set(BUILD_COMMAND ${NMAKE})
    set(CONFIG_SUFFIX .bat)
    list(APPEND QT_OPTIONS -platform win32-msvc2017)
    list(APPEND PLATFORM_OPTIONS -opengl desktop -mp)
elseif(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (HOST MATCHES "x86_64-unknown-linux-gnu")
        set(PLATFORM -platform linux-g++-cluster)
    elseif (CMAKE_COMPILER_IS_GNUCXX)
        set(PLATFORM -platform linux-g++)
    else ()
        set(PLATFORM -platform linux-clang)
    endif ()
    list(APPEND PLATFORM_OPTIONS -no-pch -c++std c++1z)
    #-device-option CROSS_COMPILE=${CROSS}
elseif(VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    vcpkg_replace_string(${SOURCE_PATH}/${PACKAGE_NAME}/qtbase/mkspecs/macx-clang/qmake.conf
        "QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.11"
        "QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14"
    )
    list(APPEND PLATFORM_OPTIONS -no-pch -c++std c++1z -sdk macosx10.14)
elseif (MINGW AND (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin"))
    set(PLATFORM -xplatform win32-g++)
    list(APPEND PLATFORM_OPTIONS -opengl desktop)
elseif (MINGW)
    set(PLATFORM -platform win32-g++)
    list(APPEND PLATFORM_OPTIONS -opengl desktop)
    set(EXEC_COMMAND sh)
endif()

if (${VCPKG_CRT_LINKAGE} STREQUAL "static")
    list(APPEND QT_OPTIONS -static-runtime)
endif ()

list(APPEND QT_OPTIONS ${PLATFORM} ${PLATFORM_OPTIONS})

set(QT_OPTIONS_REL
    ${QT_OPTIONS}
    -release
    -L ${CURRENT_INSTALLED_DIR}/lib
    -prefix ${CURRENT_INSTALLED_DIR}
    -extprefix ${CURRENT_PACKAGES_DIR}
    -hostbindir ${CURRENT_INSTALLED_DIR}/tools
    -archdatadir ${CURRENT_INSTALLED_DIR}/share/qt5
    -datadir ${CURRENT_INSTALLED_DIR}/share/qt5
    -plugindir ${CURRENT_INSTALLED_DIR}/share/qt5/plugins
    -qmldir ${CURRENT_INSTALLED_DIR}/share/qt5/qml
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
    -plugindir ${CURRENT_INSTALLED_DIR}/debug/share/qt5/plugins
    -qmldir ${CURRENT_INSTALLED_DIR}/debug/share/qt5/qml
    -headerdir ${CURRENT_INSTALLED_DIR}/include
)

file(REMOVE_RECURSE ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg)

message(STATUS "Configuring ${TARGET_TRIPLET}-dbg")
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
    ${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig
    ${CURRENT_PACKAGES_DIR}/debug/lib/cmake
    ${CURRENT_PACKAGES_DIR}/debug/share
    ${CURRENT_PACKAGES_DIR}/bin)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/${PACKAGE_NAME}/LICENSE.LGPLv3 DESTINATION  ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)