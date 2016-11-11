find_dependency(NAME GTEST LIBNAMES gmock DEBUGLIBNAMES gmock_d)

include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/..)

set (CMAKE_INCLUDE_CURRENT_DIR ON)


set (TEST_SOURCES 
    RioObsProviderParserTest.cpp
    RioObsProviderTest.cpp
    IRCELMeteoProviderTest.cpp
    StationInfoProviderTest.cpp
    TextGridProviderTest.cpp
)

if (hdf5_FOUND)
    list (APPEND TEST_SOURCES ForecastBufferTest.cpp)
endif ()

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
    rioobsprovider
    ircelmeteoprovider
    stationinfoprovider
    textgridprovider
    ${GTEST_LIBRARY}
)

if (hdf5_FOUND)
target_link_libraries(dataplugintest
    hdf5buffer
)
endif ()

add_test(NAME dataplugintest COMMAND dataplugintest)