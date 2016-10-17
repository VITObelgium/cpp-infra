if (STATIC_PLUGINS)
    set (PLUGIN_TYPE STATIC)
else ()
    set (PLUGIN_TYPE MODULE)
endif ()

if (hdf5_FOUND)
    add_library(hdf5buffer ${PLUGIN_TYPE}
        Hdf5Buffer.h
        Hdf5Buffer.cpp
        Hdf5Tools.h
        Hdf5Tools.cpp
    )

    target_link_libraries(hdf5buffer opaqcore hdf5_cpp-static)
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

add_library(asciiforecastwriter ${PLUGIN_TYPE}
    AsciiForecastWriter.h
    AsciiForecastWriter.cpp
)

add_library(sqlbuffer ${PLUGIN_TYPE}
    SqlBuffer.h
    SqlBuffer.cpp
    PredictionScheme.h
    PredictionDatabase.h
    PredictionDatabase.cpp
)

add_library(ircelmeteoprovider ${PLUGIN_TYPE}
    IRCELMeteoProvider.h
    IRCELMeteoProvider.cpp
)

add_library(rioobsprovider ${PLUGIN_TYPE}
    ObsParser.h
    ObsParser.cpp
    RioObsProvider.h
    RioObsProvider.cpp
)

add_library(xmlaqnetprovider ${PLUGIN_TYPE}
    XMLAQNetProvider.h
    XMLAQNetProvider.cpp
)

add_library(xmlgridprovider ${PLUGIN_TYPE}
    XmlGridProvider.h
    XmlGridProvider.cpp
)

target_link_libraries(sqlbuffer opaqcore ${SQLPP_CONNECTOR_LIBRARY} ${SQLITE_LIBRARY})
target_link_libraries(asciiforecastwriter opaqcore)
target_link_libraries(ircelmeteoprovider opaqcore)
target_link_libraries(rioobsprovider opaqcore)
target_link_libraries(xmlgridprovider opaqcore)
target_link_libraries(xmlaqnetprovider opaqcore)

add_subdirectory(test)