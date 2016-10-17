enable_testing()

find_library(GTEST_LIBRARY NAMES gmock)
find_library(GTEST_LIBRARY_DEBUG NAMES gmock_d)
if (GTEST_LIBRARY AND GTEST_LIBRARY_DEBUG)
    set(GTEST_LIBRARY optimized ${GTEST_LIBRARY} debug ${GTEST_LIBRARY_DEBUG})
endif ()

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