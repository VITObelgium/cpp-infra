enable_testing()

find_dependency(NAME GTEST LIBNAMES gmock DEBUGLIBNAMES gmock_d)

include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(opaqcoretest
    DateTimeTest.cpp
    main.cpp
)

target_link_libraries(opaqcoretest opaqcore ${GTEST_LIBRARY})

add_test(NAME opaqcoretest COMMAND opaqcoretest)