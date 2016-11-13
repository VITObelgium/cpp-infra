SET(MAJOR 1)
SET(MINOR 10)
SET(REVISION 0-patch1)
SET(NAME hdf5)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

IF (UNIX AND NOT APPLE)
    SET(EXTRA_FLAGS -DCMAKE_C_FLAGS=-DH5_HAVE_SYS_FILE_H=1 -DCMAKE_CXX_FLAGS=-DH5_HAVE_SYS_FILE_H=1)
ENDIF ()

IF (MINGW)
    SET (PATCH_CMD patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/mingw-timezone.patch)

    IF (CMAKE_CROSSCOMPILING)
        # hdf5 creates binaries during configure which are then executed to obtain output
        # this is not possible when cross compiling, so inject the values in the cache
        # and apply the necessary patches to avoid calling the binaries
        SET(EXTRA_FLAGS -C ${CMAKE_CURRENT_SOURCE_DIR}/TryRunResults-mingw.cmake)
        SET (PATCH_CMD ${PATCH_CMD} && patch -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/mingw-crosscompile.patch)
        SET (PATCH_CMD ${PATCH_CMD} && ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/H5Tinit.c ${CMAKE_CURRENT_BINARY_DIR}/hdf5-prefix/src/hdf5-build/H5Tinit.c)
        SET (PATCH_CMD ${PATCH_CMD} && ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/H5lib_settings.c ${CMAKE_CURRENT_BINARY_DIR}/hdf5-prefix/src/hdf5-build/H5lib_settings.c)
    ENDIF ()
ENDIF ()

EXTERNALPROJECT_ADD(${NAME}
    DEPENDS zlib
    URL https://support.hdfgroup.org/ftp/HDF5/releases/${NAME}-${MAJOR}.${MINOR}/${PACKAGE}/src/${PACKAGE}.tar.bz2 TIMEOUT 360
    PATCH_COMMAND "${PATCH_CMD}"
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS -G ${CMAKE_GENERATOR}
               -DBUILD_SHARED_LIBS=OFF
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
               -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/../local
               -DBUILD_TESTING=OFF
               -DHDF5_BUILD_TOOLS=OFF
               -DHDF5_BUILD_EXAMPLES=OFF
               -DHDF5_BUILD_HL_LIB=OFF
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
               ${EXTRA_FLAGS}
)