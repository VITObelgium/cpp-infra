set(MAJOR 2)
set(MINOR 4)
set(REVISION 0)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(PACKAGE_NAME ${PORT}-${VERSION})
set(PACKAGE ${PACKAGE_NAME}.tar.xz)
set(GDAL_VERSION_LIB "204")
set(SHA_SUM d4eb6535043b1495f691ab96aa8087d9254aa01efbc57a4051f8b9f4f6b2537719d7bf03ff82c3f6cfd0499a973c491fa9da9f5854dbd9863a0ec9796d3642bb)

if (TRIPLET_SYSTEM_ARCH MATCHES "arm")
    message(FATAL_ERROR "ARM is currently not supported.")
endif()

include(vcpkg_common_functions)

vcpkg_download_distfile(ARCHIVE
    URLS "http://download.osgeo.org/${PORT}/${VERSION}/${PACKAGE}"
    FILENAME "${PACKAGE}"
    SHA512 ${SHA_SUM}
)

# Extract source into architecture specific directory, because GDALs' build currently does not
# support out of source builds.
set(SOURCE_PATH_DEBUG   ${CURRENT_BUILDTREES_DIR}/src-${TARGET_TRIPLET}-debug/${PACKAGE_NAME})
set(SOURCE_PATH_RELEASE ${CURRENT_BUILDTREES_DIR}/src-${TARGET_TRIPLET}-release/${PACKAGE_NAME})
set(PREFIX_PATH ${CURRENT_INSTALLED_DIR})

TEST_FEATURE("geos" WITH_GEOS)
TEST_FEATURE("jpeg" WITH_JPEG)
TEST_FEATURE("gif" WITH_GIF)
TEST_FEATURE("sqlite" WITH_SQLITE)
TEST_FEATURE("expat" WITH_EXPAT)
TEST_FEATURE("pcraster" WITH_PCRASTER)
TEST_FEATURE("tools" WITH_TOOLS)

if(NOT WITH_TOOLS)
    list(APPEND OPTIONAL_PATCHES ${CMAKE_CURRENT_LIST_DIR}/optional-tools-autotools.patch)
endif()

if (VCPKG_CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    list(APPEND OPTIONAL_PATCHES ${CMAKE_CURRENT_LIST_DIR}/emscripten-longlong.patch)
endif ()

foreach(BUILD_TYPE debug release)
    file(REMOVE_RECURSE ${CURRENT_BUILDTREES_DIR}/src-${TARGET_TRIPLET}-${BUILD_TYPE})
    set(SOURCE_PATH ${CURRENT_BUILDTREES_DIR}/src-${TARGET_TRIPLET}-${BUILD_TYPE}/${PACKAGE_NAME})
    vcpkg_extract_source_archive(${ARCHIVE} ${CURRENT_BUILDTREES_DIR}/src-${TARGET_TRIPLET}-${BUILD_TYPE})
    vcpkg_apply_patches(
        SOURCE_PATH ${SOURCE_PATH}
        PATCHES
            ${CMAKE_CURRENT_LIST_DIR}/0001-Fix-debug-crt-flags.patch
            ${CMAKE_CURRENT_LIST_DIR}/optional-tools.patch
            ${CMAKE_CURRENT_LIST_DIR}/geos-link.patch
            ${CMAKE_CURRENT_LIST_DIR}/ngw.patch # fixed in the next release
            ${OPTIONAL_PATCHES}
    )

    # make sure the prefix define points to the installed dir and not the buildtree
    vcpkg_replace_string(
        ${SOURCE_PATH}/configure
        "GDAL_PREFIX=\${prefix}"
        "GDAL_PREFIX=${CURRENT_INSTALLED_DIR}"
    )

    if (MINGW)
        # don't use absolute paths for linking, the command line becomes too long
        vcpkg_replace_string(
            ${SOURCE_PATH}/GNUmakefile
            "$(GDAL_ROOT)"
            "."
        )
    endif ()

endforeach()

if (UNIX OR MINGW)
    set(CONF_CACHE_OVERRIDES)
    if(WITH_GEOS)
        list(APPEND AUTOCONF_OPTIONS --with-geos=${PREFIX_PATH}/tools/geos-config)
        # gdals geos detection detection script fails as it does not link the c++ library
        # causing linker errors for static builds
        list(APPEND CONF_CACHE_OVERRIDES ac_cv_lib_geos_c_GEOSversion=yes)
    else ()
        list(APPEND AUTOCONF_OPTIONS --without-geos)
    endif()

    if(WITH_JPEG)
        list(APPEND AUTOCONF_OPTIONS --with-jpeg=${PREFIX_PATH})
    else ()
        list(APPEND AUTOCONF_OPTIONS --without-jpeg)
    endif()

    if(WITH_GIF)
        list(APPEND AUTOCONF_OPTIONS --with-gif=${PREFIX_PATH})
    else ()
        list(APPEND AUTOCONF_OPTIONS --without-gif)
    endif()

    if(WITH_SQLITE)
        list(APPEND AUTOCONF_OPTIONS --with-sqlite3)
    else ()
        list(APPEND AUTOCONF_OPTIONS --without-sqlite3)
    endif()

    if(WITH_EXPAT)
        list(APPEND AUTOCONF_OPTIONS --with-expat=${PREFIX_PATH})
    else ()
        list(APPEND AUTOCONF_OPTIONS --without-expat)
    endif()

    if (WITH_PCRASTER)
        list(APPEND AUTOCONF_OPTIONS --with-pcraster)
    else ()
        list(APPEND AUTOCONF_OPTIONS --without-pcraster)
    endif ()

    #if("png" IN_LIST FEATURES)
        list(APPEND AUTOCONF_OPTIONS --with-png=${PREFIX_PATH})
    #else ()
    #    list(APPEND AUTOCONF_OPTIONS --without-png)
    #endif()

    if (CMAKE_CXX_VISIBILITY_PRESET STREQUAL hidden)
        list(APPEND AUTOCONF_OPTIONS --with-hide-internal-symbols=yes)
    else ()
        list(APPEND AUTOCONF_OPTIONS --with-hide-internal-symbols=no)
    endif ()

    if (VCPKG_LTO)
        list(APPEND AUTOCONF_OPTIONS --enable-lto)
    endif ()

    if (VCPKG_CMAKE_SYSTEM_NAME STREQUAL Emscripten)
        list(APPEND CONF_CACHE_OVERRIDES
            ac_cv_func_posix_spawnp=no
            ac_cv_func_vfork=no
            ac_cv_sizeof_int=4
            ac_cv_sizeof_unsigned_long=4
            ac_cv_sizeof_voidp=4
        )

        list(APPEND AUTOCONF_OPTIONS
            --without-threads
            --without-sse
            --without-ssse3
        )
    else ()
        list(APPEND AUTOCONF_OPTIONS --with-threads)
    endif ()

    if (NOT MINGW)
        list(APPEND AUTOCONF_OPTIONS --with-sysroot=${PREFIX_PATH})
    endif ()

    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)

    vcpkg_execute_required_process(
        COMMAND sh ./autogen.sh
        WORKING_DIRECTORY ${SOURCE_PATH_DEBUG}
        LOGNAME autogen-${TARGET_TRIPLET}-dbg
    )

    vcpkg_execute_required_process(
        COMMAND sh ./autogen.sh
        WORKING_DIRECTORY ${SOURCE_PATH_RELEASE}
        LOGNAME autogen-${TARGET_TRIPLET}-rel
    )
    
    vcpkg_configure_autoconf(
        IN_SOURCE
        CXX_STANDARD 14
        SOURCE_PATH_DEBUG ${SOURCE_PATH_DEBUG}
        SOURCE_PATH_RELEASE ${SOURCE_PATH_RELEASE}
        OPTIONS
            ${CONF_CACHE_OVERRIDES}
            ac_cv_header_lzma_h=yes
            ac_cv_lib_lzma_lzma_code=yes
            am_cv_func_iconv=no
            --without-ld-shared
            --with-cpp14
            --without-libtool
            --with-static-proj4=${PREFIX_PATH}
            --with-libz=${PREFIX_PATH}
            --with-libtiff=${PREFIX_PATH}
            --with-geotiff=internal
            --with-liblzma=yes
            --with-libjson-c=internal
            --without-pg
            --without-grass
            --without-libgrass
            --without-cfitsio
            --without-netcdf
            --without-jpeg
            --without-ogdi
            --without-fme
            --without-freexl
            --without-hdf4
            --without-hdf5
            --without-jasper
            --without-ecw
            --without-kakadu
            --without-mrsid
            --without-jp2mrsid
            --without-bsb
            --without-grib
            --without-mysql
            --without-ingres
            --without-xerces
            --without-xml2
            --without-mrf
            --without-odbc
            --without-curl
            --without-sqlite3
            --without-idb
            --without-sde
            --without-perl
            --without-pcre
            --without-php
            --without-python
            --without-webp
            --without-gnm
            ${AUTOCONF_OPTIONS}
        )

    vcpkg_build_autotools(IN_SOURCE)
    vcpkg_install_autotools(IN_SOURCE)

    vcpkg_replace_string(${CURRENT_PACKAGES_DIR}/bin/gdal-config "packages/${PORT}_${TARGET_TRIPLET}" "installed/${TARGET_TRIPLET}")


    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/share)

    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/bin)

    file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/tools)
    if(WITH_TOOLS)
        file(GLOB TOOL_FILES ${CURRENT_PACKAGES_DIR}/bin/*)
        file(COPY ${TOOL_FILES} DESTINATION ${CURRENT_PACKAGES_DIR}/tools)
    else ()
        file(COPY ${CURRENT_PACKAGES_DIR}/bin/gdal-config DESTINATION ${CURRENT_PACKAGES_DIR}/tools)
    endif()
    file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/bin)

    vcpkg_fixup_pkgconfig_file()
else ()

find_program(NMAKE nmake)
if (NOT NMAKE)
    message(FATAL_ERROR "Could not find nmake build tool")
endif ()

if (VCPKG_LIBRARY_LINKAGE STREQUAL static)
    foreach(BUILD_TYPE debug release)
        if (BUILD_TYPE MATCHES debug)
            set(GDAL_SRC_ROOT ${SOURCE_PATH_DEBUG})
        else ()
            set(GDAL_SRC_ROOT ${SOURCE_PATH_RELEASE})
        endif ()

        message(STATUS "Patching GDAL for configured link lib: ${GDAL_SRC_ROOT}")

        file(READ ${GDAL_SRC_ROOT}/makefile.vc conf_file)
        # don't install the dll
        string(REPLACE "install: $(GDAL_DLL) plugin_dir apps_dir " "install: plugin_dir apps_dir " conf_file "${conf_file}")
        # copy static lib instead of import lib during install
        string(REPLACE "copy gdal_i.lib $(LIBDIR)" "copy $(GDALLIB) $(LIBDIR)" conf_file "${conf_file}")
        file(WRITE ${GDAL_SRC_ROOT}/makefile.vc "${conf_file}")

        file(READ ${GDAL_SRC_ROOT}/apps/makefile.vc conf_file)
        string(REPLACE "LIBS	=	$(GDALLIB)" "LIBS    =   $(GDALLIB) $(EXTERNAL_LIBS)" conf_file "${conf_file}")
        file(WRITE ${GDAL_SRC_ROOT}/apps/makefile.vc "${conf_file}")

        SET (MAKE_FILES
            apps/makefile.vc
            frmts/grib/makefile.vc
            frmts/mrsid/makefile.vc
            frmts/mrsid_lidar/makefile.vc
            frmts/sde/makefile.vc
            ogr/ogrsf_frmts/amigocloud/makefile.vc
            ogr/ogrsf_frmts/arcobjects/makefile.vc
            ogr/ogrsf_frmts/dwg/makefile.vc
            ogr/ogrsf_frmts/filegdb/makefile.vc
            ogr/ogrsf_frmts/libkml/makefile.vc
            ogr/ogrsf_frmts/mongodb/makefile.vc
            ogr/ogrsf_frmts/oci/makefile.vc
        )

        foreach(make_file IN LISTS MAKE_FILES)
            file(READ ${GDAL_SRC_ROOT}/${make_file} conf_file)
            # link applications against static lib and extenal libs instead of import lib
            string(REPLACE "$(GDAL_ROOT)\\gdal_i.lib" "$(GDALLIB) $(EXTERNAL_LIBS)" conf_file "${conf_file}")
            string(REPLACE "$(GDAL_ROOT)/gdal_i.lib" "$(GDALLIB) $(EXTERNAL_LIBS)" conf_file "${conf_file}")
            file(WRITE ${GDAL_SRC_ROOT}/${make_file} ${conf_file})
        endforeach()
    endforeach()
endif ()

vcpkg_replace_string(${SOURCE_PATH_DEBUG}/nmake.opt "INCLUDE_GNM_FRMTS = YES" "#INCLUDE_GNM_FRMTS = YES")
vcpkg_replace_string(${SOURCE_PATH_RELEASE}/nmake.opt "INCLUDE_GNM_FRMTS = YES" "#INCLUDE_GNM_FRMTS = YES")
# don't provide a default value for jpeg support
# vcpkg_replace_string(${SOURCE_PATH_DEBUG}/nmake.opt "JPEG_SUPPORTED = 1" "")
# vcpkg_replace_string(${SOURCE_PATH_RELEASE}/nmake.opt "JPEG_SUPPORTED = 1" "")
# vcpkg_replace_string(${SOURCE_PATH_DEBUG}/nmake.opt "JPEG12_SUPPORTED = 1" "")
# vcpkg_replace_string(${SOURCE_PATH_RELEASE}/nmake.opt "JPEG12_SUPPORTED = 1" "")

file(TO_NATIVE_PATH "${CURRENT_PACKAGES_DIR}" NATIVE_PACKAGES_DIR)
file(TO_NATIVE_PATH "${CURRENT_PACKAGES_DIR}/share/gdal" NATIVE_DATA_DIR)
file(TO_NATIVE_PATH "${CURRENT_PACKAGES_DIR}/share/gdal/html" NATIVE_HTML_DIR)

# Setup proj4 libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" PROJ_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/proj_5_2.lib" PROJ_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/proj_5_2_d.lib" PROJ_LIBRARY_DBG)

# Setup libpng libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" PNG_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/libpng.lib" PNG_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/libpngd.lib" PNG_LIBRARY_DBG)

# Setup jpeg libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" JPEG_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/jpeg.lib" JPEG_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/jpegd.lib" JPEG_LIBRARY_DBG)

# Setup geos libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" GEOS_INCLUDE_DIR)
if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    # the static build of the geos library includes the capi symbols
    file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/libgeos.lib" GEOS_LIBRARY_REL)
    file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/libgeos.lib" GEOS_LIBRARY_DBG)
else ()
    file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/libgeos_c.lib" GEOS_LIBRARY_REL)
    file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/libgeos_cd.lib" GEOS_LIBRARY_DBG)
endif ()

# Setup expat libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" EXPAT_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/expat.lib" EXPAT_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/expatd.lib" EXPAT_LIBRARY_DBG)

# Setup curl libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" CURL_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/libcurl.lib" CURL_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/libcurl.lib" CURL_LIBRARY_DBG)

# Setup sqlite3 libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" SQLITE_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/sqlite3.lib" SQLITE_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/sqlite3d.lib" SQLITE_LIBRARY_DBG)

# Setup MySQL libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include/mysql" MYSQL_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/libmysql.lib" MYSQL_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/libmysql.lib" MYSQL_LIBRARY_DBG)

# Setup PostgreSQL libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" PGSQL_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/libpq.lib" PGSQL_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/libpqd.lib" PGSQL_LIBRARY_DBG)

# Setup WebP libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" WEBP_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/webp.lib" WEBP_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/webpd.lib" WEBP_LIBRARY_DBG)

# Setup libxml2 libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" XML2_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/libxml2.lib" XML2_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/libxml2.lib" XML2_LIBRARY_DBG)

# Setup liblzma libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" LZMA_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/lzma.lib" LZMA_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/lzmad.lib" LZMA_LIBRARY_DBG)

# Setup zlib libraries + include path
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/include" ZLIB_INCLUDE_DIR)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/lib/zlib.lib" ZLIB_LIBRARY_REL)
file(TO_NATIVE_PATH "${CURRENT_INSTALLED_DIR}/debug/lib/zlibd.lib" ZLIB_LIBRARY_DBG)

set(NMAKE_OPTIONS
    GDAL_HOME=${NATIVE_PACKAGES_DIR}
    DATADIR=${NATIVE_DATA_DIR}
    HTMLDIR=${NATIVE_HTML_DIR}
    PROJ_INCLUDE=-I${PROJ_INCLUDE_DIR}
    "PROJ_FLAGS=-DPROJ_STATIC -DPROJ_VERSION=5"
    #CURL_INC=-I${CURL_INCLUDE_DIR}
    #MYSQL_INC_DIR=${MYSQL_INCLUDE_DIR}
    #PG_INC_DIR=${PGSQL_INCLUDE_DIR}
    #WEBP_ENABLED=YES
    #WEBP_CFLAGS=-I${WEBP_INCLUDE_DIR}
    #LIBXML2_INC=-I${XML2_INCLUDE_DIR}
    PNG_EXTERNAL_LIB=1
    PNGDIR=${PNG_INCLUDE_DIR}
    BSB_SUPPORTED=NO
    ODBC_SUPPORTED=0
    #JPEG12_SUPPORTED=0
    TIFF_OPTS=-DBIGTIFF_SUPPORT
    MSVC_VER=1915
    ZLIB_EXTERNAL_LIB=1
    ZLIB_INC=-I${ZLIB_INCLUDE_DIR}
    LZMA_CFLAGS=-I${LZMA_INCLUDE_DIR}
)

if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
    list(APPEND NMAKE_OPTIONS WIN64=1)
endif()

if (VCPKG_CRT_LINKAGE STREQUAL static)
    set(LINKAGE_FLAGS "/MT")
else()
    set(LINKAGE_FLAGS "/MD")
endif()

if (VCPKG_LIBRARY_LINKAGE STREQUAL dynamic)
    list(APPEND NMAKE_OPTIONS DLLBUILD=1)
else ()
    list(APPEND NMAKE_OPTIONS DLLBUILD=0)
endif()

if(WITH_TOOLS)
    list(APPEND NMAKE_OPTIONS BUILD_TOOLS=1)
endif ()

if(WITH_GEOS)
    list(APPEND NMAKE_OPTIONS
        GEOS_DIR=${GEOS_INCLUDE_DIR}
        "GEOS_CFLAGS=-I${GEOS_INCLUDE_DIR} -DHAVE_GEOS"
    )

    list(APPEND NMAKE_OPTIONS_DBG GEOS_LIB=${GEOS_LIBRARY_DBG})
    list(APPEND NMAKE_OPTIONS_REL GEOS_LIB=${GEOS_LIBRARY_REL})
endif()

if(WITH_EXPAT)
    list(APPEND NMAKE_OPTIONS
        EXPAT_DIR=${CURRENT_INSTALLED_DIR}
        EXPAT_INCLUDE=-I${EXPAT_INCLUDE_DIR}
    )

    list(APPEND NMAKE_OPTIONS_DBG EXPAT_LIB=${EXPAT_LIBRARY_DBG})
    list(APPEND NMAKE_OPTIONS_REL EXPAT_LIB=${EXPAT_LIBRARY_REL})
endif()


# if("jpeg" IN_LIST FEATURES)
#     list(APPEND NMAKE_OPTIONS
#         JPEG_SUPPORTED=1
#         JPEG_EXTERNAL_LIB=1
#         JPEGDIR=${JPEG_INCLUDE_DIR}
#     )

#     list(APPEND NMAKE_OPTIONS_DBG JPEG_LIB=${JPEG_LIBRARY_DBG})
#     list(APPEND NMAKE_OPTIONS_REL JPEG_LIB=${JPEG_LIBRARY_REL})
# else ()
#     list(APPEND NMAKE_OPTIONS
#         JPEG12_SUPPORTED=0
#     )
# endif()

# if("gif" IN_LIST FEATURES)
#     list(APPEND NMAKE_OPTIONS
#         OPENJPEG_ENABLED=YES
#         OPENJPEG_CFLAGS=-I${OPENJPEG_INCLUDE_DIR}
#         OPENJPEG_VERSION=20100
#     )

#     list(APPEND NMAKE_OPTIONS_DBG OPENJPEG_LIB=${OPENJPEG_LIBRARY_DBG})
#     list(APPEND NMAKE_OPTIONS_REL OPENJPEG_LIB=${OPENJPEG_LIBRARY_REL})
# endif()

if("sqlite" IN_LIST FEATURES)
    list(APPEND NMAKE_OPTIONS SQLITE_INC=-I${SQLITE_INCLUDE_DIR})
    list(APPEND NMAKE_OPTIONS_DBG SQLITE_LIB=${SQLITE_LIBRARY_DBG})
    list(APPEND NMAKE_OPTIONS_REL SQLITE_LIB=${SQLITE_LIBRARY_REL})
endif()

set(NMAKE_OPTIONS_REL
    "${NMAKE_OPTIONS}"
    CXX_CRT_FLAGS=${LINKAGE_FLAGS}
    PROJ_LIBRARY=${PROJ_LIBRARY_REL}
    PNG_LIB=${PNG_LIBRARY_REL}
    #"CURL_LIB=${CURL_LIBRARY_REL} wsock32.lib wldap32.lib winmm.lib"
    #MYSQL_LIB=${MYSQL_LIBRARY_REL}
    #PG_LIB=${PGSQL_LIBRARY_REL}
    #WEBP_LIBS=${WEBP_LIBRARY_REL}
    #LIBXML2_LIB=${XML2_LIBRARY_REL}
    ZLIB_LIB=${ZLIB_LIBRARY_REL}
    LZMA_LIBS=${LZMA_LIBRARY_REL}
)

set(NMAKE_OPTIONS_DBG
    "${NMAKE_OPTIONS}"
    CXX_CRT_FLAGS="${LINKAGE_FLAGS}d"
    PROJ_LIBRARY=${PROJ_LIBRARY_DBG}
    PNG_LIB=${PNG_LIBRARY_DBG}
    #"CURL_LIB=${CURL_LIBRARY_DBG} wsock32.lib wldap32.lib winmm.lib"
    #MYSQL_LIB=${MYSQL_LIBRARY_DBG}
    #PG_LIB=${PGSQL_LIBRARY_DBG}
    #WEBP_LIBS=${WEBP_LIBRARY_DBG}
    #LIBXML2_LIB=${XML2_LIBRARY_DBG}
    ZLIB_LIB=${ZLIB_LIBRARY_DBG}
    LZMA_LIBS=${LZMA_LIBRARY_DBG}
    DEBUG=1
    WITH_PDB=1
    GDAL_PDB=${SOURCE_PATH_DEBUG}/gdal.pdb
)

################
# Release build
################
message(STATUS "Building ${TARGET_TRIPLET}-rel")
if (VCPKG_VERBOSE)
STRING(REPLACE ";;" ";" PRINT_COMMAND "${NMAKE} /G -f makefile.vc ${NMAKE_OPTIONS_REL}")
STRING(REPLACE ";" "\n   " PRINT_COMMAND "${PRINT_COMMAND}")
message(STATUS "Release Command: ${PRINT_COMMAND}")
endif ()
vcpkg_execute_required_process(
    COMMAND ${NMAKE} -f makefile.vc
    "${NMAKE_OPTIONS_REL}"
    WORKING_DIRECTORY ${SOURCE_PATH_RELEASE}
    LOGNAME nmake-build-${TARGET_TRIPLET}-release
)

################
# Debug build
################
message(STATUS "Building ${TARGET_TRIPLET}-dbg")
if (VCPKG_VERBOSE)
    STRING(REPLACE ";;" ";" PRINT_COMMAND "${NMAKE} /G -f makefile.vc ${NMAKE_OPTIONS_DBG}")
    STRING(REPLACE ";" "\n   " PRINT_COMMAND "${PRINT_COMMAND}")
    message(STATUS "Debug Command: ${PRINT_COMMAND}")
endif ()
vcpkg_execute_required_process(
  COMMAND ${NMAKE} /G -f makefile.vc
  "${NMAKE_OPTIONS_DBG}"
  WORKING_DIRECTORY ${SOURCE_PATH_DEBUG}
  LOGNAME nmake-build-${TARGET_TRIPLET}-debug
)

message(STATUS "Packaging ${TARGET_TRIPLET}")
file(MAKE_DIRECTORY ${CURRENT_PACKAGES_DIR}/share/gdal/html)

vcpkg_execute_required_process(
  COMMAND ${NMAKE} -f makefile.vc
  "${NMAKE_OPTIONS_REL}"
  "install"
  "devinstall"
  WORKING_DIRECTORY ${SOURCE_PATH_RELEASE}
  LOGNAME nmake-install-${TARGET_TRIPLET}
)

if (VCPKG_LIBRARY_LINKAGE STREQUAL static)
  file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/bin)
  file(COPY ${SOURCE_PATH_DEBUG}/gdal.lib   DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib)
  file(COPY ${SOURCE_PATH_DEBUG}/gdal.pdb DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib)
  file(COPY ${SOURCE_PATH_RELEASE}/gdal.lib DESTINATION ${CURRENT_PACKAGES_DIR}/lib)
  file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/gdal.lib ${CURRENT_PACKAGES_DIR}/debug/lib/gdald.lib)
else()
  file(GLOB EXE_FILES ${CURRENT_PACKAGES_DIR}/bin/*.exe)
  file(REMOVE ${EXE_FILES} ${CURRENT_PACKAGES_DIR}/lib/gdal.lib)
  file(COPY ${SOURCE_PATH_DEBUG}/gdal${GDAL_VERSION_LIB}.dll DESTINATION ${CURRENT_PACKAGES_DIR}/debug/bin)
  file(COPY ${SOURCE_PATH_DEBUG}/gdal_i.lib DESTINATION ${CURRENT_PACKAGES_DIR}/debug/lib)
  file(RENAME ${CURRENT_PACKAGES_DIR}/lib/gdal_i.lib ${CURRENT_PACKAGES_DIR}/lib/gdal.lib)
  file(RENAME ${CURRENT_PACKAGES_DIR}/debug/lib/gdal_i.lib ${CURRENT_PACKAGES_DIR}/debug/lib/gdald.lib)
endif()

endif ()

vcpkg_copy_pdbs()

configure_file(${CMAKE_CURRENT_LIST_DIR}/FindGdal.cmake.in ${CURRENT_PACKAGES_DIR}/share/cmake/FindGdal.cmake @ONLY)

# Handle copyright
file(INSTALL ${CURRENT_PACKAGES_DIR}/share/gdal/LICENSE.TXT DESTINATION ${CURRENT_PACKAGES_DIR}/share/gdal RENAME copyright)
