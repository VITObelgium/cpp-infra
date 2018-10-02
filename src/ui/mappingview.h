#pragma once

#include "Aggregation.h"
#include "ConfigurationHandler.h"
#include "DateTime.h"
#include "Engine.h"
#include "PollutantManager.h"
#include "plugins/PluginFactory.h"

#include "ui_mappingview.h"

#include <QComboBox>
#include <QStandardItemModel>
#include <QWidget>
#include <memory>

namespace opaq {

namespace config {
struct Component;
}

class Station;
class ForecastBuffer;

class MappingView : public QWidget
{
    Q_OBJECT

public:
    explicit MappingView(QWidget* parent = nullptr);
    virtual ~MappingView();

private:
    void showConfigFileSelector();
    void loadConfiguration(const QString& path);
    void loadRecentConfigurations();
    void updateRecentConfiguration(const QString& filePath);

    void updateStationModel(const std::vector<Station>& stations);
    void updatePollutantModel();

    Ui::MappingView _ui;
    QStandardItemModel _stationModel;
    QStandardItemModel _pollutantModel;
    QStandardItemModel _aggregationModel;

    config::PollutantManager _pollutantMgr;
    ConfigurationHandler _config;
    PluginFactory _pluginFactory;
    Engine _engine;
};
}
