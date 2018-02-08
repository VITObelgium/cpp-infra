find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

add_executable(uiinfratest
    simpletreemodel.h
    fixeditemproxymodeltest.cpp
    main.cpp
)

target_link_libraries(uiinfratest
    infra
    uiinfra
    GTest::GTest
    GMock::GMock
)

set_target_properties(uiinfratest PROPERTIES AUTOMOC ON)

gtest_add_tests(TARGET uiinfratest)
