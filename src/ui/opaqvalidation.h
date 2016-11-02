#pragma once

#include "DateTime.h"
#include "Aggregation.h"
#include "validationresultsmodel.h"
#include "ui_opaqvalidation.h"

#include <QWidget>

namespace opaq
{

namespace Config
{
struct Component;
}

class Engine;
class ConfigurationHandler;

class OpaqValidation : public QWidget
{
    Q_OBJECT

public:
    explicit OpaqValidation(QWidget *parent = 0);

    void setConfig(ConfigurationHandler& config);
    void setEngine(Engine& engine);

    void setModels(const std::vector<Config::Component>& models);
    void setStationModel(QAbstractItemModel& model);
    void setPollutantModel(QAbstractItemModel& model);
    void setAggregationModel(QAbstractItemModel& model);

private:
    void runValidation();

    std::string pollutant() const noexcept;
    Aggregation::Type aggregation() const noexcept;
    std::string station() const noexcept;
    chrono::date_time startTime() const noexcept;
    chrono::date_time endTime() const noexcept;
    chrono::days forecastHorizon() const noexcept;

    Ui::OpaqValidation _ui;

    ConfigurationHandler* _config;
    Engine* _engine;
    ValidationResultsModel _model;
};

}
