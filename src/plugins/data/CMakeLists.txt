if (hdf5_FOUND)

add_library(hdf5buffer MODULE
    Hdf5Buffer.h
    Hdf5Buffer.cpp
    Hdf5Tools.h
    Hdf5Tools.cpp
)

target_link_libraries(hdf5buffer opaqcore hdf5_cpp-static)

endif ()

add_library(asciiforecastwriter MODULE
    AsciiForecastWriter.h
    AsciiForecastWriter.cpp
)

add_library(ircelmeteoprovider MODULE
    IRCELMeteoProvider.h
    IRCELMeteoProvider.cpp
)

add_library(rioobsprovider MODULE
    ObsParser.h
    ObsParser.cpp
    RioObsProvider.h
    RioObsProvider.cpp
)

add_library(xmlaqnetprovider MODULE
    XMLAQNetProvider.h
    XMLAQNetProvider.cpp
)

add_library(xmlgridprovider MODULE
    XmlGridProvider.h
    XmlGridProvider.cpp
)

target_link_libraries(asciiforecastwriter opaqcore)
target_link_libraries(ircelmeteoprovider opaqcore)
target_link_libraries(rioobsprovider opaqcore)
target_link_libraries(xmlgridprovider opaqcore)
target_link_libraries(xmlaqnetprovider opaqcore)

add_subdirectory(test)