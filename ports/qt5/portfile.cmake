include(vcpkg_common_functions)

set(MAJOR 5)
set(MINOR 12)
set(REVISION 1)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(RELEASE official)
if (WIN32)
    set(TARBALL_EXTENSION zip)
    set (SHASUM bf78a5bd055b94a0dfca55f134bbfb99b16fa3c08945c3e23c0ba0a34061cfc362d2f33f19846abdceac6c4ef20a95986420abea23692013ba8f8d631adfbc7d)
else ()
    set(TARBALL_EXTENSION tar.xz)
    set (SHASUM 2b25f460d3ad0bab72645b0b30c6dab38f638f9f09640ff24812bafd9b9cfaea657abd303bbbf5689ea79d0779377d6f24a15b535f047afc7525b8cc4f1acd98)
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

set (OPTIONAL_ARGS)
if(NOT "tools" IN_LIST FEATURES)
    list(APPEND OPTIONAL_SKIPS -skip qttools)
endif ()

if(NOT "qml" IN_LIST FEATURES)
    list(APPEND OPTIONAL_ARGS -skip qtquickcontrols -skip qtquickcontrols2 -skip qtdeclarative)
endif ()

vcpkg_download_distfile(ARCHIVE
    URLS "http://download.qt.io/archive/qt/${MAJOR}.${MINOR}/${VERSION}/single/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 ${SHASUM}
)

vcpkg_extract_source_archive_ex(
    ARCHIVE ${ARCHIVE}
    REF ${PORT}
    OUT_SOURCE_PATH SOURCE_PATH
    PATCHES
        cmake-debug-plugin-name-osx.patch
        support-static-builds-with-cmake.patch
        support-static-builds-with-cmake-qtplugins.patch
        mingw-cmake-prl-file-has-no-lib-prefix.patch
        zlib.patch
        jpeg.patch
        png.patch
        sqlite3.patch
        pcre2.patch
        freetype.patch
        harfbuzz.patch
 )

if (MINGW AND NOT CMAKE_CROSSCOMPILING)
    set (FILES_TO_FIX
        qtbase/include/QtEventDispatcherSupport/${VERSION}/QtEventDispatcherSupport/private/qwindowsguieventdispatcher_p.h
        qtbase/include/QtFontDatabaseSupport/${VERSION}/QtFontDatabaseSupport/private/qwindowsfontdatabase_ft_p.h
        qtbase/include/QtWindowsUIAutomationSupport/${VERSION}/QtWindowsUIAutomationSupport/private/qwindowsuiawrapper_p.h
    )

    foreach(FILE_TO_FIX IN LISTS FILES_TO_FIX)
        # fix for include going wrong on native mingw build
        vcpkg_replace_string(${SOURCE_PATH}/${FILE_TO_FIX}
            "../../../../../"
            "../../../../"
        )
    endforeach()
endif ()

# copy the g++-cluster compiler specification
file(COPY ${CMAKE_CURRENT_LIST_DIR}/linux-g++-cluster DESTINATION ${SOURCE_PATH}/qtbase/mkspecs)

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
    list(APPEND OPTIONAL_ARGS -sql-sqlite -system-sqlite -no-sql-sqlite2 -no-sql-mysql -no-sql-psql -no-sql-db2 -no-sql-tds)
endif()

if("ssl" IN_LIST FEATURES)
    list(APPEND OPTIONAL_ARGS -ssl)
    if (VCPKG_LIBRARY_LINKAGE STREQUAL "static")
        list(APPEND OPTIONAL_ARGS -openssl-linked)
        if((VCPKG_CMAKE_SYSTEM_NAME STREQUAL "WindowsStore" OR NOT DEFINED VCPKG_CMAKE_SYSTEM_NAME) AND NOT MINGW)
            list(APPEND OPTIONAL_ARGS "OPENSSL_LIBS=${CURRENT_INSTALLED_DIR}/lib/libeay32.lib ${CURRENT_INSTALLED_DIR}/lib/ssleay32.lib Gdi32.lib Advapi32.lib User32.lib Ws2_32.lib")
        endif ()
    endif()
else ()
    list(APPEND OPTIONAL_ARGS -no-openssl)
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

    if("angle" IN_LIST FEATURES)
        list(APPEND PLATFORM_OPTIONS -angle)
    else ()
        list(APPEND PLATFORM_OPTIONS -opengl desktop)
    endif()

    set(CONFIG_SUFFIX .bat)
    list(APPEND QT_OPTIONS -platform win32-msvc2017)
    list(APPEND PLATFORM_OPTIONS -mp)
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
    list(APPEND PLATFORM_OPTIONS -c++std c++14 -no-pch)

    vcpkg_replace_string(${SOURCE_PATH}/qtbase/mkspecs/common/macx.conf
        "QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.12"
        "QMAKE_MACOSX_DEPLOYMENT_TARGET = ${CMAKE_OSX_DEPLOYMENT_TARGET}"
    )
elseif (MINGW AND (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin"))
    set(PLATFORM -xplatform win32-g++)
    list(APPEND PLATFORM_OPTIONS -opengl desktop)
elseif (MINGW)
    set (DIRECTX_SDK_DIR "c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/")
    if (NOT EXISTS ${DIRECTX_SDK_DIR})
        message (FATAL_ERROR "The DirectX SDK needs to be installed in: ${DIRECTX_SDK_DIR}")
    endif ()

    set(BUILD_COMMAND jom)
    set(ENV{DXSDK_DIR} "${DIRECTX_SDK_DIR}")
    vcpkg_replace_string(${SOURCE_PATH}/qtbase/src/angle/src/common/common.pri "Utilities\\\\bin\\\\x64\\\\fxc.exe" "Utilities/bin/x64/fxc.exe")
    vcpkg_replace_string(${SOURCE_PATH}/qtbase/src/angle/src/common/common.pri "Utilities\\\\bin\\\\x86\\\\fxc.exe" "Utilities/bin/x86/fxc.exe")

    set(PLATFORM -platform win32-g++)
    list(APPEND PLATFORM_OPTIONS -angle)
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
    -plugindir ${CURRENT_PACKAGES_DIR}/plugins
    -qmldir ${CURRENT_PACKAGES_DIR}/qml
)

set(QT_OPTIONS_DBG
    ${QT_OPTIONS}
    -debug
    -L ${CURRENT_INSTALLED_DIR}/debug/lib
    -prefix ${CURRENT_INSTALLED_DIR}/debug
    -extprefix ${CURRENT_PACKAGES_DIR}/debug
    -hostbindir ${CURRENT_PACKAGES_DIR}/debug/tools
    -archdatadir ${CURRENT_PACKAGES_DIR}/share/qt5/debug
    -datadir ${CURRENT_PACKAGES_DIR}/share/qt5/debug
    -plugindir ${CURRENT_PACKAGES_DIR}/debug/plugins
    -qmldir ${CURRENT_PACKAGES_DIR}/debug/qml
    -headerdir ${CURRENT_PACKAGES_DIR}/debug/include
)

file(REMOVE_RECURSE ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg)

set(ENV{PKG_CONFIG_LIBDIR} "${CURRENT_INSTALLED_DIR}/lib/pkgconfig")
message(STATUS "Configuring ${TARGET_TRIPLET}-dbg")
if(VCPKG_VERBOSE)
    STRING(JOIN " " QT_ARGS ${QT_OPTIONS_DBG})
    message(STATUS "${SOURCE_PATH}/configure${CONFIG_SUFFIX} ${QT_ARGS}")
endif()
file(MAKE_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg)
vcpkg_execute_required_process(
    COMMAND ${EXEC_COMMAND} ${SOURCE_PATH}/configure${CONFIG_SUFFIX} ${QT_OPTIONS_DBG}
    WORKING_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg
    LOGNAME qt-configure-${TARGET_TRIPLET}-debug
)

message(STATUS "Configuring ${TARGET_TRIPLET}-rel")
message(STATUS "${QT_OPTIONS_REL}")
file(MAKE_DIRECTORY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel)
vcpkg_execute_required_process(
    COMMAND ${EXEC_COMMAND} ${SOURCE_PATH}/configure${CONFIG_SUFFIX} ${QT_OPTIONS_REL}
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

if (EXISTS ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/qtbase/bin/qmake.exe)
    # qt-bug: file does not get installed
    file(COPY ${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/qtbase/bin/qmake.exe DESTINATION ${CURRENT_PACKAGES_DIR}/tools)
else ()
    MESSAGE (STATUS "Please remove this code if printed on windows: Fix no longer necessary")
endif ()

# Fix the cmake files
file(RENAME ${CURRENT_PACKAGES_DIR}/lib/cmake ${CURRENT_PACKAGES_DIR}/share/cmake)
vcpkg_execute_required_process(
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/fixcmake.py
    WORKING_DIRECTORY ${CURRENT_PACKAGES_DIR}/share/cmake
    LOGNAME fix-cmake
)

if("location" IN_LIST FEATURES)
    vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/share/cmake/Qt5Location/Qt5LocationConfig.cmake "Qt5Location_*Plugin.cmake" "Qt5Location_*Factory*.cmake")
endif()

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
    ${CURRENT_PACKAGES_DIR}/debug/lib/Qt5Bootstrap.prl
    ${CURRENT_PACKAGES_DIR}/lib/Qt5Bootstrap.lib
    ${CURRENT_PACKAGES_DIR}/lib/Qt5Bootstrap.prl
)

file(INSTALL ${CMAKE_CURRENT_LIST_DIR}/FindQtPlugin.cmake DESTINATION ${CURRENT_PACKAGES_DIR}/share/cmake)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE.LGPLv3 DESTINATION  ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)
