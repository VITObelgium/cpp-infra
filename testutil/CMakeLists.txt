find_package(GTest REQUIRED)

add_library(infratestutil
    include/infra/test/printsupport.h printsupport.cpp
)

target_include_directories(infratestutil PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(infratestutil PUBLIC infra GTest::GTest)
