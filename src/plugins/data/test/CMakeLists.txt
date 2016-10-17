enable_testing()

find_dependency(NAME GTEST LIBNAMES gmock DEBUGLIBNAMES gmock_d)

include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(dataplugintest
    ../ObsParser.cpp
    ../PredictionDatabase.cpp
    rioobsprovidertest.cpp
    SqlBufferTest.cpp
    main.cpp
)

target_link_libraries(dataplugintest opaqcore ${GTEST_LIBRARY} ${SQLPP_CONNECTOR_LIBRARY} ${SQLITE_LIBRARY})

add_test(NAME dataplugintest COMMAND dataplugintest)