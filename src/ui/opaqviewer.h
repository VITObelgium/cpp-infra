#pragma once

#include "DateTime.h"
#include "Aggregation.h"

#include "stationresultsmodel.h"
#include "ui_opaqviewer.h"

#include <string>
#include <QWidget>

namespace opaq
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

    void setModels(const std::vector<config::Component>& models);
    void setStationModel(QAbstractItemModel& model);
    void setPollutantModel(QAbstractItemModel& model);
    void setAggregationModel(QAbstractItemModel& model);

    void setForecastBuffer(ForecastBuffer& buffer);
    void resetForecastBuffer();

    void setForecastHorizon(chrono::days fcHor);

private:
    void updateResultsForCurrentStation();
    void runSimulation();

    std::string pollutant() const noexcept;
    Aggregation::Type aggregation() const noexcept;
    chrono::date_time basetime() const noexcept;
    std::string station() const noexcept;

    Ui::OpaqViewer _ui;
    StationResultsModel _model;

    chrono::days _forecastHorizon;
    ForecastBuffer* _buffer;
    ConfigurationHandler* _config;
    Engine* _engine;
};
}
