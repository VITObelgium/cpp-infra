add_executable(infratest
    filesystemtest.cpp
)

if (INFRA_GDAL)
    target_sources(infratest PRIVATE
        gdaltest.cpp
    )
endif ()

target_link_libraries(infratest
    infra
    GTest::GTest
    GTest::Main
)

target_compile_definitions(infratest PRIVATE -DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

add_test(NAME infratest COMMAND infratest)
