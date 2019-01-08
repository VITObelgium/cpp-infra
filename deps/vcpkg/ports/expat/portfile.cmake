set(MAJOR 2)
set(MINOR 2)
set(REVISION 6)
set(VERSION R_${MAJOR}_${MINOR}_${REVISION})

include(vcpkg_common_functions)
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO libexpat/libexpat
    REF ${VERSION}
    SHA512 727fbd24041c9af71b32353448583a6d8b38ddf93b10c97510e847939c2ad2be9b40ff6e6e27b725bff277982c2fc96c75f19c4a3ac4a246133eb62870c963d8
    HEAD_REF master
)

if(VCPKG_CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    # fix arc4random implicit declaration when using emscripten
    message(STATUS "Applying arc4random patch")
    vcpkg_replace_string(${SOURCE_PATH}/expat/lib/xmlparse.c "#include <stdio.h>" "unsigned int arc4random(void);\n#include <stdio.h>")
endif()

if(VCPKG_LIBRARY_LINKAGE STREQUAL dynamic)
    set(EXPAT_LINKAGE ON)
else()
    set(EXPAT_LINKAGE OFF)
endif()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}/expat
    PREFER_NINJA
    OPTIONS
        -DBUILD_examples=OFF
        -DBUILD_tests=OFF
        -DBUILD_tools=OFF
        -DBUILD_shared=${EXPAT_LINKAGE}
)

vcpkg_install_cmake()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include ${CURRENT_PACKAGES_DIR}/debug/lib/pkgconfig)
file(INSTALL ${SOURCE_PATH}/expat/COPYING DESTINATION ${CURRENT_PACKAGES_DIR}/share/expat RENAME copyright)

vcpkg_fixup_pkgconfig_file()
vcpkg_copy_pdbs()

file(READ ${CURRENT_PACKAGES_DIR}/include/expat_external.h EXPAT_EXTERNAL_H)
if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    string(REPLACE "!defined(XML_STATIC)" "/* vcpkg static build !defined(XML_STATIC) */ 0" EXPAT_EXTERNAL_H "${EXPAT_EXTERNAL_H}")
endif()
file(WRITE ${CURRENT_PACKAGES_DIR}/include/expat_external.h "${EXPAT_EXTERNAL_H}")
