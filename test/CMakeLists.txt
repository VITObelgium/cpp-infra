find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

add_executable(infratest
    casttest.cpp
    colormaptest.cpp
    inireadertest.cpp
    filesystemtest.cpp
    signaltest.cpp
    stringtest.cpp
    threadpooltest.cpp
    main.cpp
)

if (INFRA_XML)
    target_sources(infratest PRIVATE
        configdocumenttest.cpp
    )
endif ()


if (INFRA_GDAL)
    target_sources(infratest PRIVATE
        gdaltest.cpp
        gdalgeometrytest.cpp
    )

    target_compile_definitions(infratest PRIVATE HAVE_GDAL)
endif ()

target_link_libraries(infratest
    infra
    GTest::GTest
    GMock::GMock
)

target_compile_definitions(infratest PRIVATE -DTEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

add_test(NAME infratest COMMAND infratest)
