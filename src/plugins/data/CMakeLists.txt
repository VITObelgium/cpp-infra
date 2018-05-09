find_package(hdf5 CONFIG REQUIRED)
find_package(Sqlpp11)

add_library(hdf5tools STATIC
    Hdf5Tools.h
    Hdf5Tools.cpp
)

add_library(hdf5buffer STATIC
    Hdf5Buffer.h
    Hdf5Buffer.cpp
)

add_library(riooutputbuffer STATIC
    RioOutputBuffer.h
    RioOutputBuffer.cpp
)

add_library(asciiforecastwriter STATIC
    AsciiForecastWriter.h
    AsciiForecastWriter.cpp
)

add_library(sqlbuffer STATIC
    SqlBuffer.h
    SqlBuffer.cpp
    PredictionScheme.h
    PredictionDatabase.h
    PredictionDatabase.cpp
)

add_library(ircelmeteoprovider STATIC
    IRCELMeteoProvider.h
    IRCELMeteoProvider.cpp
)

add_library(rioobsprovider STATIC
    ObsParser.h
    ObsParser.cpp
    RioObsProvider.h
    RioObsProvider.cpp
)

add_library(xmlaqnetprovider STATIC
    XMLAQNetProvider.h
    XMLAQNetProvider.cpp
)

add_library(xmlgridprovider STATIC
    XmlGridProvider.h
    XmlGridProvider.cpp
)

add_library(textgridprovider STATIC
    TextGridProvider.h
    TextGridProvider.cpp
)

add_library(stationinfoprovider STATIC
    StationInfoProvider.h
    StationInfoProvider.cpp
)

target_link_libraries(hdf5tools hdf5::hdf5_cpp-static)
target_link_libraries(hdf5buffer opaqcore hdf5tools)
target_link_libraries(riooutputbuffer opaqcore hdf5tools)
target_link_libraries(sqlbuffer opaqcore Sqlpp11::Sqlpp11 Sqlpp11::Sqlite3)
target_link_libraries(rioobsprovider opaqcore)
target_link_libraries(asciiforecastwriter opaqcore)
target_link_libraries(ircelmeteoprovider opaqcore)
target_link_libraries(xmlgridprovider opaqcore)
target_link_libraries(textgridprovider opaqcore)
target_link_libraries(xmlaqnetprovider opaqcore)
target_link_libraries(stationinfoprovider opaqcore)

set (PLUGINS ${PLUGINS} sqlbuffer hdf5buffer riooutputbuffer asciiforecastwriter ircelmeteoprovider rioobsprovider xmlgridprovider textgridprovider xmlaqnetprovider stationinfoprovider PARENT_SCOPE)

add_subdirectory(test)