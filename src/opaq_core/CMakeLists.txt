set (CORE_SOURCES
    Aggregation.h
    Aggregation.cpp
    AQNetwork.h
    AQNetwork.cpp
    AQNetworkProvider.h
    Cell.h
    Cell.cpp
    Component.h
    Component.cpp
    ComponentManagerFactory.h
    ComponentManagerFactory.cpp
    ComponentManager.h
    ComponentManager.cpp
    ComponentManagerDynamic.h
    ComponentManagerDynamic.cpp
    ComponentManagerStatic.h
    ComponentManagerStatic.cpp
    ConfigurationHandler.h
    ConfigurationHandler.cpp
    DateTime.h
    DateTime.cpp
    Engine.h
    Engine.cpp
    Exceptions.h
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
    TimeSeries.h
)

set (CORE_CONFIG_SOURCES
    config/Component.h
    config/OpaqRun.h
    config/OpaqRun.cpp
    config/Plugin.h
    config/ForecastStage.h
    config/ForecastStage.cpp
    config/MappingStage.h
    config/MappingStage.cpp
)

set (CORE_DATA_SOURCES
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

set (CORE_TOOLS_SOURCES
    tools/AQNetworkTools.h
    tools/AQNetworkTools.cpp
    tools/DateTimeTools.h
    tools/DateTimeTools.cpp
    tools/ExceptionTools.h
    tools/FileTools.h
    tools/FileTools.cpp
    tools/GzipReader.h
    tools/GzipReader.cpp
    tools/StringTools.h
    tools/StringTools.cpp
    tools/XmlTools.h
    tools/XmlTools.cpp
    tools/CalcTools.h
)

source_group(Config FILES ${CORE_CONFIG_SOURCES})
source_group(Tools FILES ${CORE_TOOLS_SOURCES})
source_group(Data FILES ${CORE_DATA_SOURCES})

add_library(opaqcore ${CORE_SOURCES} ${CORE_CONFIG_SOURCES} ${CORE_TOOLS_SOURCES} ${CORE_DATA_SOURCES})
target_include_directories(opaqcore INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(opaqcore fmt ${Boost_LIBRARIES} ${TINYXML_LIBRARY} ${ZLIB_LIBRARY} ${EXTRA_LIBS} ${CMAKE_DL_LIBS})
set_target_properties(opaqcore PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_executable(opaq opaq.cpp)
target_link_libraries(opaq opaqcore)
