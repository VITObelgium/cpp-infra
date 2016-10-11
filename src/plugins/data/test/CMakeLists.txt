enable_testing()

find_library(GTEST_LIBRARY NAMES gmock)
find_library(GTEST_LIBRARY_DEBUG NAMES gmock_d)

include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(dataplugintest
    ../obsparser.cpp
    rioobsprovidertest.cpp
    main.cpp
)

target_link_libraries(dataplugintest opaqcore)

if (WIN32)
    target_link_libraries(dataplugintest debug ${GTEST_LIBRARY_DEBUG} optimized ${GTEST_LIBRARY})
else ()
    target_link_libraries(dataplugintest ${GTEST_LIBRARY})
endif ()

add_test(NAME dataplugintest COMMAND dataplugintest)