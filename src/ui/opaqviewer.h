#pragma once

#include "DateTime.h"
#include "Aggregation.h"

#include "stationresultsmodel.h"
#include "ui_opaqviewer.h"

#include <string>
#include <QWidget>

namespace OPAQ
{

class Engine;
class ConfigurationHandler;

class OpaqViewer : public QWidget
{
    Q_OBJECT

public:
    explicit OpaqViewer(QWidget* parent = 0);

    void setConfig(ConfigurationHandler& config);
    void setEngine(Engine& engine);

    void setModels(const std::vector<Config::Component>& models);
    void setStationModel(QAbstractItemModel& model);
    void setPollutantModel(QAbstractItemModel& model);
    void setAggregationModel(QAbstractItemModel& model);

    void setForecastBuffer(ForecastBuffer& buffer);
    void setForecastHorizon(days fcHor);

private:
    void updateResultsForCurrentStation();
    void runSimulation();

    std::string pollutant() const noexcept;
    Aggregation::Type aggregation() const noexcept;
    std::string basetime() const noexcept;
    std::string station() const noexcept;

    Ui::OpaqViewer _ui;
    StationResultsModel _model;

    days _forecastHorizon;
    ForecastBuffer* _buffer;
    ConfigurationHandler* _config;
    Engine* _engine;
};
}
