find_package(doctest CONFIG REQUIRED)

add_library(infratestutil
    include/infra/test/printsupport.h printsupport.cpp
    include/infra/test/reporter.h
)

target_include_directories(infratestutil PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(infratestutil PUBLIC infra doctest::doctest)
