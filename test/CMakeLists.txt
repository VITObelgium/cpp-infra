find_package(trompeloeil CONFIG REQUIRED)

add_executable(infratest
    casttest.cpp
    colortest.cpp
	chronotest.cpp
    colormaptest.cpp
    interpolatetest.cpp
    inireadertest.cpp
    filesystemtest.cpp
    filelocktest.cpp
    mathtest.cpp
    signaltest.cpp
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
        legenddataanalysertest.cpp
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
        cryptotest.cpp
    )
endif ()

target_link_libraries(infratest
    Infra::infra
	Infra::infratestutil
    Infra::infratestutilmain
    trompeloeil::trompeloeil
)

target_compile_definitions(infratest PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

add_test(NAME infratest COMMAND infratest)
