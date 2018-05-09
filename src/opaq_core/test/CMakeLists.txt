find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(opaqcoretest
    DateTimeTest.cpp
    TimeSeriesTest.cpp
    ComponentManagerTest.cpp
    main.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

target_link_libraries(opaqcoretest opaqcore GTest::GTest GMock::GMock)
target_link_libraries(opaqcoretest ${PLUGINS})

add_test(NAME opaqcore COMMAND opaqcoretest)

