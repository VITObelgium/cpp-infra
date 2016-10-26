find_dependency(NAME GTEST LIBNAMES gmock DEBUGLIBNAMES gmock_d)

include_directories(SYSTEM ${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(opaqcoretest
    DateTimeTest.cpp
    ComponentManagerTest.cpp
    main.cpp
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)

target_link_libraries(opaqcoretest opaqcore ${GTEST_LIBRARY})
if (STATIC_PLUGINS)
    target_link_libraries(opaqcoretest ${PLUGINS})
endif ()

add_test(NAME opaqcore COMMAND opaqcoretest)

