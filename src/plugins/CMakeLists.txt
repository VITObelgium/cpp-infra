find_package(hdf5 CONFIG REQUIRED)
find_package(Sqlpp11 REQUIRED)

add_library(opaqplugins
    PluginFactory.h
    PluginFactory.cpp

    data/Hdf5Tools.h
    data/Hdf5Tools.cpp
    data/Hdf5Buffer.h
    data/Hdf5Buffer.cpp
    data/RioOutputBuffer.h
    data/RioOutputBuffer.cpp
    data/AsciiForecastWriter.h
    data/AsciiForecastWriter.cpp
    data/SqlBuffer.h
    data/SqlBuffer.cpp
    data/PredictionScheme.h
    data/PredictionDatabase.h
    data/PredictionDatabaseInterface.h
    data/PredictionDatabaseFactory.h
    data/PredictionDatabaseFactory.cpp
    data/IRCELMeteoProvider.h
    data/IRCELMeteoProvider.cpp
    data/ObsParser.h
    data/ObsParser.cpp
    data/RioObsProvider.h
    data/RioObsProvider.cpp
    data/XMLAQNetProvider.h
    data/XMLAQNetProvider.cpp
    data/XmlGridProvider.h
    data/XmlGridProvider.cpp
    data/TextGridProvider.h
    data/TextGridProvider.cpp
    data/StationInfoProvider.h
    data/StationInfoProvider.cpp

    # mpl_feedforward
    forecast/MLP_FeedForwardModel.h
    forecast/MLP_FeedForwardModel.cpp
    forecast/scaler.h
    forecast/scaler.cpp
    forecast/layer.h
    forecast/layer.cpp
    forecast/feedforwardnet.h
    forecast/feedforwardnet.cpp
    forecast/transfcn.h
    # ovl_ircel_model1
    forecast/OVL_IRCEL_model1.h
    forecast/OVL_IRCEL_model1.cpp
    # ovl_ircel_model2
    forecast/OVL_IRCEL_model2.h
    forecast/OVL_IRCEL_model2.cpp
    # ovl_ircel_model3
    forecast/OVL_IRCEL_model3.h
    forecast/OVL_IRCEL_model3.cpp
    # ovl
    forecast/OVL.h
    forecast/OVL.cpp

    mapping/InverseDistanceWeighting.h
    mapping/InverseDistanceWeighting.cpp
)

target_link_libraries(opaqplugins PRIVATE
    opaqcore
    hdf5::hdf5_cpp-static
    Sqlpp11::Sqlpp11
    Sqlpp11::Sqlite3
    Eigen3::Eigen
    infra
)

if (OPAQ_ENABLE_POSTGRESQL)
    if (NOT TARGET Sqlpp11::Postgresql)
        message(FATAL_ERROR "Postgresql enabled but the library could not be found")
    endif ()

    target_link_libraries(opaqplugins PRIVATE Sqlpp11::Postgresql)
endif ()

add_subdirectory(data/test)
