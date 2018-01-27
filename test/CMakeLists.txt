add_executable(infratest
    filesystemtest.cpp
    colormaptest.cpp
    stringtest.cpp
    inireadertest.cpp
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
)

target_compile_definitions(infratest PRIVATE -DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

gtest_discover_tests(infratest)
