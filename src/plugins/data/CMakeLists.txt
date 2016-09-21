ADD_LIBRARY(asciiforecastwriter MODULE
    AsciiForecastWriter.h
    AsciiForecastWriter.cpp
)

ADD_LIBRARY(hdf5buffer MODULE
    Hdf5Buffer.h
    Hdf5Buffer.cpp
)

ADD_LIBRARY(ircelmeteoprovider MODULE
    IRCELMeteoProvider.h
    IRCELMeteoProvider.cpp
)

ADD_LIBRARY(rioobsprovider MODULE
    RioObsProvider.h
    RioObsProvider.cpp
)

ADD_LIBRARY(xmlaqnetprovider MODULE
    XMLAQNetProvider.h
    XMLAQNetProvider.cpp
)

ADD_LIBRARY(xmlgridprovider MODULE
    XmlGridProvider.h
    XmlGridProvider.cpp
)

TARGET_LINK_LIBRARIES(hdf5buffer opaqcore)
TARGET_LINK_LIBRARIES(hdf5buffer optimized ${HDF5_CPP_LIBRARY})
TARGET_LINK_LIBRARIES(hdf5buffer debug ${HDF5_CPP_LIBRARY_DEBUG})

TARGET_LINK_LIBRARIES(asciiforecastwriter opaqcore)
TARGET_LINK_LIBRARIES(ircelmeteoprovider opaqcore)
TARGET_LINK_LIBRARIES(rioobsprovider opaqcore)
TARGET_LINK_LIBRARIES(xmlgridprovider opaqcore)
TARGET_LINK_LIBRARIES(xmlaqnetprovider opaqcore)