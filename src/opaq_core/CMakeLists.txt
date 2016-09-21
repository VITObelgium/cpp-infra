SET (CORE_SOURCES
    Aggregation.h
    Aggregation.cpp
    AQNetwork.h
    AQNetwork.cpp
    AQNetworkProvider.h
    AQNetworkProvider.cpp
    Cell.h
    Cell.cpp
    Component.h
    Component.cpp
    ComponentManager.h
    ComponentManager.cpp
    ConfigurationHandler.h
    ConfigurationHandler.cpp
    DateTime.h
    DateTime.cpp
    Engine.h
    Engine.cpp
    Grid.h
    Grid.cpp
    OpaqDllExports.h
    OpaqMath.h
    OpaqMath.cpp
    Model.cpp
    Logger.h
    Logger.cpp
    Point.h
    Point.cpp
    Pollutant.h
    Pollutant.cpp
    PollutantManager.h
    PollutantManager.cpp
    Station.h
    Station.cpp
    TimeInterval.h
    TimeInterval.cpp
)

SET (CORE_CONFIG_SOURCES
    config/Component.h
    config/Component.cpp
    config/OpaqRun.h
    config/OpaqRun.cpp
    config/Plugin.h
    config/Plugin.cpp
    config/ForecastStage.h
    config/ForecastStage.cpp
    config/MappingStage.h
    config/MappingStage.cpp
)

SET (CORE_DATA_SOURCES
    data/ForecastBuffer.cpp
    data/ForecastBuffer.h
    data/DataProvider.h
    data/DataProvider.cpp
    data/MeteoProvider.h
    data/MeteoProvider.cpp
    data/GridProvider.h
    data/GridProvider.cpp
    data/ForecastOutputWriter.h
    data/ForecastOutputWriter.cpp
)

SET (CORE_TOOLS_SOURCES
    tools/AQNetworkTools.h
    tools/AQNetworkTools.cpp
    tools/DateTimeTools.h
    tools/DateTimeTools.cpp
    tools/FileTools.h
    tools/FileTools.cpp
    tools/GzipReader.h
    tools/GzipReader.cpp
    tools/Hdf5Tools.cpp
    tools/Hdf5Tools.h
    tools/StringTools.h
    tools/StringTools.cpp
    tools/XmlTools.h
    tools/XmlTools.cpp
)

SOURCE_GROUP(Config FILES ${CORE_CONFIG_SOURCES})
SOURCE_GROUP(Tools FILES ${CORE_TOOLS_SOURCES})
SOURCE_GROUP(Data FILES ${CORE_DATA_SOURCES})

ADD_LIBRARY(opaqcore ${CORE_SOURCES} ${CORE_CONFIG_SOURCES} ${CORE_TOOLS_SOURCES} ${CORE_DATA_SOURCES})

TARGET_LINK_LIBRARIES(opaqcore ${EXTRA_LIBS} ${CMAKE_DL_LIBS})

TARGET_LINK_LIBRARIES(opaqcore optimized ${ZLIB_LIBRARY})
TARGET_LINK_LIBRARIES(opaqcore optimized ${GZSTREAM_LIBRARY})
TARGET_LINK_LIBRARIES(opaqcore optimized ${HDF5_LIBRARY})
TARGET_LINK_LIBRARIES(opaqcore optimized ${TINYXML_LIBRARY})
TARGET_LINK_LIBRARIES(opaqcore optimized ${DL_LIBRARY})

TARGET_LINK_LIBRARIES(opaqcore debug ${ZLIB_LIBRARY_DEBUG})
TARGET_LINK_LIBRARIES(opaqcore debug ${GZSTREAM_LIBRARY_DEBUG})
TARGET_LINK_LIBRARIES(opaqcore debug ${HDF5_LIBRARY_DEBUG})
TARGET_LINK_LIBRARIES(opaqcore debug ${TINYXML_LIBRARY_DEBUG})
TARGET_LINK_LIBRARIES(opaqcore debug ${DL_LIBRARY_DEBUG})


ADD_EXECUTABLE(opaq opaq.cpp)
TARGET_LINK_LIBRARIES(opaq opaqcore)
TARGET_LINK_LIBRARIES(opaq optimized ${GETOPT_LIBRARY})
TARGET_LINK_LIBRARIES(opaq debug ${GETOPT_LIBRARY_DEBUG})
