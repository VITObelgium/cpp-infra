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

target_link_libraries(hdf5buffer opaqcore hdf5tools hdf5_cpp-static)
target_link_libraries(riooutputbuffer opaqcore hdf5tools hdf5_cpp-static)
target_link_libraries(sqlbuffer opaqcore Sqlpp11::Sqlpp11 Sqlpp11::Sqlite3)
target_link_libraries(rioobsprovider opaqcore)
target_link_libraries(asciiforecastwriter opaqcore)
target_link_libraries(ircelmeteoprovider opaqcore)
target_link_libraries(xmlgridprovider opaqcore)
target_link_libraries(textgridprovider opaqcore)
target_link_libraries(xmlaqnetprovider opaqcore)
target_link_libraries(stationinfoprovider opaqcore)

if (NOT STATIC_PLUGINS)
    set_target_properties(hdf5tools PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(hdf5buffer PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(riooutputbuffer PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(sqlbuffer PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(rioobsprovider PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(asciiforecastwriter PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(ircelmeteoprovider PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(xmlgridprovider PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(xmlaqnetprovider PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(stationinfoprovider PROPERTIES POSITION_INDEPENDENT_CODE ON)

    add_library(hdf5bufferplugin            MODULE Hdf5BufferPlugin.cpp)
    add_library(riooutputbufferplugin       MODULE Hdf5BufferPlugin.cpp)
    add_library(sqlbufferplugin             MODULE SqlBufferPlugin.cpp)
    add_library(rioobsproviderplugin        MODULE RioObsProviderPlugin.cpp)
    add_library(asciiforecastwriterplugin   MODULE AsciiForecastWriterPlugin.cpp)
    add_library(ircelmeteoproviderplugin    MODULE IRCELMeteoProviderPlugin.cpp)
    add_library(xmlgridproviderplugin       MODULE XmlGridProviderPlugin.cpp)
    add_library(textgridproviderplugin      MODULE TextGridProviderPlugin.cpp)
    add_library(xmlaqnetproviderplugin      MODULE XMLAQNetProviderPlugin.cpp)
    add_library(stationinfoproviderplugin   MODULE StationInfoProviderPlugin.cpp)

    target_link_libraries(hdf5bufferplugin hdf5buffer)
    target_link_libraries(riooutputbufferplugin riooutputbuffer)
    target_link_libraries(sqlbufferplugin sqlbuffer)
    target_link_libraries(rioobsproviderplugin rioobsprovider)
    target_link_libraries(asciiforecastwriterplugin asciiforecastwriter)
    target_link_libraries(ircelmeteoproviderplugin ircelmeteoprovider)
    target_link_libraries(xmlgridproviderplugin xmlgridprovider)
    target_link_libraries(textgridproviderplugin textgridprovider)
    target_link_libraries(xmlaqnetproviderplugin xmlaqnetprovider)
    target_link_libraries(stationinfoproviderplugin stationinfoprovider)
endif ()

set (PLUGINS ${PLUGINS} hdf5buffer riooutputbuffer sqlbuffer asciiforecastwriter ircelmeteoprovider rioobsprovider xmlgridprovider textgridprovider xmlaqnetprovider stationinfoprovider PARENT_SCOPE)

add_subdirectory(test)