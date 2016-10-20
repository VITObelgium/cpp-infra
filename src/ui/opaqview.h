#pragma once

#include "DateTime.h"
#include "Aggregation.h"
#include "ConfigurationHandler.h"
#include "Engine.h"

#include "ui_opaqview.h"

#include <QComboBox>
#include <QStandardItemModel>
#include <QWidget>
#include <memory>

namespace OPAQ
{

namespace Config
{
struct Component;
}

class Station;
class ForecastBuffer;

class OpaqView : public QWidget
{
    Q_OBJECT

public:
    explicit OpaqView(QWidget* parent = nullptr);
    virtual ~OpaqView();

private:
    void setModels(const std::vector<Config::Component>& models);

    void showConfigFileSelector();
    void loadConfiguration(const QString& path);
    void loadRecentConfigurations();
    void updateRecentConfiguration(const QString &filePath);
    
    void updateStationModel(const std::vector<std::unique_ptr<Station>>& stations);
    void updatePollutantModel();

    Ui::OpaqView _ui;
    QStandardItemModel _stationModel;
    QStandardItemModel _pollutantModel;
    QStandardItemModel _aggregationModel;
    
    Config::PollutantManager _pollutantMgr;
    ConfigurationHandler _config;
    Engine _engine;
};
}
