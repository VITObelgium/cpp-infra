SET(NAME wingetopt)

EXTERNALPROJECT_ADD(${NAME}
    URL https://github.com/alex85k/${NAME}/archive/master.zip TIMEOUT 180
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    CMAKE_ARGS -G ${CMAKE_GENERATOR}
               -DCMAKE_DEBUG_POSTFIX=_d
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)