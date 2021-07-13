find_package(trompeloeil CONFIG REQUIRED)

add_executable(infratest
    casttest.cpp
    colortest.cpp
	chronotest.cpp
    colormaptest.cpp
    inireadertest.cpp
    filesystemtest.cpp
    mathtest.cpp
    signaltest.cpp
    legenddataanalysertest.cpp
    stringtest.cpp
    threadpooltest.cpp
    workerthreadtest.cpp
)

if (INFRA_XML)
    target_sources(infratest PRIVATE
        configdocumenttest.cpp
    )
endif ()

if (INFRA_CHARSET)
    target_sources(infratest PRIVATE
        charsettest.cpp
    )
endif ()

if (INFRA_GDAL)
    target_sources(infratest PRIVATE
        gdaltest.cpp
        gdalgeometrytest.cpp
        geocodertest.cpp
        geometadatatest.cpp
    )

    target_compile_definitions(infratest PRIVATE HAVE_GDAL)
endif ()

if (INFRA_GDAL AND INFRA_CHARSET)
    target_sources(infratest PRIVATE
        csvreadertest.cpp
        csvwritertest.cpp
    )
endif ()

if (INFRA_COMPRESSION)
    target_sources(infratest PRIVATE
        compressiontest.cpp
    )
endif ()

if (INFRA_HASHING)
    target_sources(infratest PRIVATE
        hashtest.cpp
    )
endif ()

target_link_libraries(infratest
    Infra::infra
	Infra::infratestutil
    Infra::infratestutilmain
    trompeloeil
)

target_compile_definitions(infratest PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

add_test(NAME infratest COMMAND infratest)
