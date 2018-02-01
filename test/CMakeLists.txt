find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

add_executable(infratest
    colormaptest.cpp
    inireadertest.cpp
    filesystemtest.cpp
    signaltest.cpp
    stringtest.cpp
    threadpooltest.cpp
    main.cpp
)

if (INFRA_GDAL)
    target_sources(infratest PRIVATE
        gdaltest.cpp
    )
endif ()

target_link_libraries(infratest
    infra
    GTest::GTest
    GMock::GMock
)

target_compile_definitions(infratest PRIVATE -DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

gtest_discover_tests(infratest)
