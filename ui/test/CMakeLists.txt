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

add_test(NAME uiinfra COMMAND uiinfratest)
if (MSVC)
    set_tests_properties(uiinfra PROPERTIES ENVIRONMENT "PATH=${Qt5_DIR}/../../../bin")
endif ()
