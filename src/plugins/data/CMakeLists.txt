if (hdf5_FOUND)
    add_library(hdf5buffer STATIC
        Hdf5Buffer.h
        Hdf5Buffer.cpp
        Hdf5Tools.h
        Hdf5Tools.cpp
    )

    add_library(hdf5bufferplugin ${PLUGIN_TYPE}
        Hdf5BufferPlugin.cpp
    )

    set_target_properties(hdf5buffer PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(hdf5buffer opaqcore hdf5_cpp-static)
    target_link_libraries(hdf5bufferplugin hdf5buffer)
endif ()

find_library(SQLITE_LIBRARY NAMES sqlite3)
find_library(SQLITE_LIBRARY_DEBUG NAMES sqlite3_d)
if (SQLITE_LIBRARY AND SQLITE_LIBRARY_DEBUG)
    set(SQLITE_LIBRARY optimized ${SQLITE_LIBRARY} debug ${SQLITE_LIBRARY_DEBUG})
endif ()

find_library(SQLPP_CONNECTOR_LIBRARY NAMES sqlpp11-connector-sqlite3)
find_library(SQLPP_CONNECTOR_LIBRARY_DEBUG NAMES sqlpp11-connector-sqlite3_d)
if (SQLPP_CONNECTOR_LIBRARY AND SQLPP_CONNECTOR_LIBRARY_DEBUG)
    set(SQLPP_CONNECTOR_LIBRARY optimized ${SQLPP_CONNECTOR_LIBRARY} debug ${SQLPP_CONNECTOR_LIBRARY_DEBUG})
endif ()

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

target_link_libraries(sqlbuffer opaqcore ${SQLPP_CONNECTOR_LIBRARY} ${SQLITE_LIBRARY})
target_link_libraries(rioobsprovider opaqcore)
target_link_libraries(asciiforecastwriter opaqcore)
target_link_libraries(ircelmeteoprovider opaqcore)
target_link_libraries(xmlgridprovider opaqcore)
target_link_libraries(textgridprovider opaqcore)
target_link_libraries(xmlaqnetprovider opaqcore)
target_link_libraries(stationinfoprovider opaqcore)

if (NOT STATIC_PLUGINS)
    set_target_properties(sqlbuffer PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(rioobsprovider PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(asciiforecastwriter PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(ircelmeteoprovider PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(xmlgridprovider PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(xmlaqnetprovider PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set_target_properties(stationinfoprovider PROPERTIES POSITION_INDEPENDENT_CODE ON)

    add_library(sqlbufferplugin             MODULE SqlBufferPlugin.cpp)
    add_library(rioobsproviderplugin        MODULE RioObsProviderPlugin.cpp)
    add_library(asciiforecastwriterplugin   MODULE AsciiForecastwriterPlugin.cpp)
    add_library(ircelmeteoproviderplugin    MODULE IRCELMeteoProviderPlugin.cpp)
    add_library(xmlgridproviderplugin       MODULE XmlGridProviderPlugin.cpp)
    add_library(textgridproviderplugin      MODULE TextGridProviderPlugin.cpp)
    add_library(xmlaqnetproviderplugin      MODULE XMLAQNetproviderPlugin.cpp)
    add_library(stationinfoproviderplugin   MODULE StationInfoProviderPlugin.cpp)

    target_link_libraries(sqlbufferplugin sqlbuffer)
    target_link_libraries(rioobsproviderplugin rioobsprovider)
    target_link_libraries(asciiforecastwriterplugin asciiforecastwriter)
    target_link_libraries(ircelmeteoproviderplugin ircelmeteoprovider)
    target_link_libraries(xmlgridproviderplugin xmlgridprovider)
    target_link_libraries(textgridproviderplugin textgridprovider)
    target_link_libraries(xmlaqnetproviderplugin xmlaqnetprovider)
    target_link_libraries(stationinfoproviderplugin stationinfoprovider)
endif ()

set (PLUGINS ${PLUGINS} hdf5buffer sqlbuffer asciiforecastwriter ircelmeteoprovider rioobsprovider xmlgridprovider textgridprovider xmlaqnetprovider stationinfoprovider PARENT_SCOPE)

add_subdirectory(test)