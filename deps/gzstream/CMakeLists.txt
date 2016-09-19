SET(MAJOR 1)
SET(MINOR 5)
SET(NAME gzstream)
SET(PACKAGE ${NAME}-${MAJOR}.${MINOR})

EXTERNALPROJECT_ADD(${NAME}
    DEPENDS zlib
    URL http://www.cs.unc.edu/Research/compgeom/${NAME}/${NAME}.tgz TIMEOUT 180
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/patch/CMakeLists.txt ${CMAKE_CURRENT_BINARY_DIR}/gzstream-prefix/src/gzstream
    CMAKE_ARGS -G ${CMAKE_GENERATOR}
               -DCMAKE_BUILD_TYPE=Release
               -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/../local
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)