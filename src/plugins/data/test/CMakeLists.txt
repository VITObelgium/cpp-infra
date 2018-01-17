find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/..)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (TEST_SOURCES
    RioObsProviderParserTest.cpp
    RioObsProviderTest.cpp
    IRCELMeteoProviderTest.cpp
    StationInfoProviderTest.cpp
    TextGridProviderTest.cpp
    ForecastBufferTest.cpp
)

set (MOCKS
    EngineMock.h
)

add_executable(dataplugintest
    main.cpp
    ${TEST_SOURCES}
    ${MOCKS}
)

source_group(Mocks FILES ${MOCKS})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

target_link_libraries(dataplugintest
    opaqcore
    sqlbuffer
    hdf5buffer
    rioobsprovider
    ircelmeteoprovider
    stationinfoprovider
    textgridprovider
    GTest::GTest
    GMock::GMock
)

add_test(NAME dataplugintest COMMAND dataplugintest)