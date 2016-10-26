find_dependency(NAME GTEST LIBNAMES gmock DEBUGLIBNAMES gmock_d)

include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/..)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(dataplugintest
    ../ObsParser.cpp
    rioobsprovidertest.cpp
    ForecastBufferTest.cpp
    main.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

target_link_libraries(dataplugintest
    opaqcore
    hdf5buffer
    sqlbuffer
    ${GTEST_LIBRARY}
)

add_test(NAME dataplugintest COMMAND dataplugintest)