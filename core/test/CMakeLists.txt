add_executable(gdxcoretest
    eigeniterationtest.cpp
    rastertest.cpp
    sparserastertest.cpp
    rasterareatest.cpp
    rasteriteratortest.cpp
    rastercomparetest.cpp
    rasteriotest.cpp
    rasterspantest.cpp
    operatorstest.cpp
    main.cpp
    testconfig.h.in
)

target_include_directories(gdxcoretest
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(gdxcoretest gdxcore gdxtestutil GTest::GTest GMock::GMock)

add_test(NAME gdxcoretest COMMAND gdxcoretest)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h)
