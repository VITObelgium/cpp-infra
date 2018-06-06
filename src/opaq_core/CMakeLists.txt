set (CMAKE_INCLUDE_CURRENT_DIR ON)

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
    ComponentManager.h
    ComponentManager.cpp
    ConfigurationHandler.h
    ConfigurationHandler.cpp
    DateTime.h
    DateTime.cpp
    Engine.h
    Engine.cpp
    Exceptions.h
    Exceptions.cpp
    GridType.h
    Grid.h
    Grid.cpp
    OpaqMath.h
    OpaqMath.cpp
    Model.cpp
    Point.h
    Point.cpp
    Pollutant.h
    Pollutant.cpp
    PollutantManager.h
    PollutantManager.cpp
    PluginFactoryInterface.h
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
    data/ForecastBuffer.h
    data/IMappingBuffer.h
    data/DataProvider.h
    data/MeteoProvider.h
    data/IGridProvider.h
    data/ForecastOutputWriter.h
    data/IStationInfoProvider.h
)

set (CORE_TOOLS_SOURCES
    tools/ExceptionTools.h
    tools/FileTools.h
    tools/FileTools.cpp
    tools/GzipReader.h
    tools/GzipReader.cpp
    tools/XmlTools.h
    tools/XmlTools.cpp
    tools/CalcTools.h
    tools/ScopeGuard.h
)

source_group(config FILES ${CORE_CONFIG_SOURCES})
source_group(tools FILES ${CORE_TOOLS_SOURCES})
source_group(data FILES ${CORE_DATA_SOURCES})

add_library(opaqcore ${CORE_SOURCES} ${CORE_CONFIG_SOURCES} ${CORE_TOOLS_SOURCES} ${CORE_DATA_SOURCES})
target_include_directories(opaqcore INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(opaqcore
PUBLIC
    infra
    fmt::fmt
    spdlog::spdlog
    Boost::disable_autolinking
PRIVATE
    Boost::system
    Boost::iostreams
    ${EXTRA_LIBS}
)

# We use our own version of FMT, let spdlog use the same version
target_compile_definitions(opaqcore PUBLIC
    SPDLOG_FMT_EXTERNAL
)

set_target_properties(opaqcore PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_subdirectory(test)
