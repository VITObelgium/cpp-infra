SET(MAJOR 1)
SET(MINOR 8)
SET(REVISION 0)
SET(NAME googletest)

EXTERNALPROJECT_ADD(${NAME}
    PREFIX ${NAME}
    URL https://github.com/google/${NAME}/archive/release-${MAJOR}.${MINOR}.${REVISION}.tar.gz TIMEOUT 30
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
               -Dgtest_force_shared_crt=ON
               ${POSTFIX_CONFIG}
)