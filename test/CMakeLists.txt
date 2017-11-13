add_executable(infratest
    filesystemtest.cpp
)

target_link_libraries(infratest
    infra
    GTest::GTest
    GTest::Main
)

add_test(NAME infratest COMMAND infratest)
