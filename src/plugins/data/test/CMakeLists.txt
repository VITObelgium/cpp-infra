find_dependency(NAME GTEST LIBNAMES gmock DEBUGLIBNAMES gmock_d)

include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/..)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (MOCKS
    EngineMock.h
)

add_executable(dataplugintest
    RioObsProviderParserTest.cpp
    RioObsProviderTest.cpp
    IRCELMeteoProviderTest.cpp
    ForecastBufferTest.cpp
    main.cpp
    ${MOCKS}
)

source_group(Mocks FILES ${MOCKS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

target_link_libraries(dataplugintest
    opaqcore
    hdf5buffer
    sqlbuffer
    rioobsprovider
    ircelmeteoprovider
    ${GTEST_LIBRARY}
)

add_test(NAME dataplugintest COMMAND dataplugintest)