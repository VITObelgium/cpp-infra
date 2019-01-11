#pragma once

#include "Aggregation.h"
#include "DateTime.h"
#include "Engine.h"
#include "PollutantManager.h"
#include "plugins/PluginFactory.h"
#include "rio.hpp"

#include "ui_mappingview.h"

#include <QComboBox>
#include <QStandardItemModel>
#include <QWidget>
#include <memory>
#include <string_view>

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

signals:
    void computeProgress(int);
    void computeSucceeded(RasterPtr);
    void computeFailed(QString message);

private:
    void showConfigFileSelector();
    void showForecastFileSelector();

    void loadConfiguration(const QString& path);
    void loadRecentConfigurations();
    void loadRecentForecasts();
    void updateRecentConfigurations(const QString& filePath);
    void updateRecentForecasts(const QString& filePath);
    void updateRecentpathsList(const QString& settingsName, const QString& filePath);

    void updateConfigurationsModel(const std::vector<std::string_view>& configurations);
    void updatePollutantModel(const std::vector<std::string_view>& pollutants);
    void updateInterpolationModel(const std::vector<std::string_view>& interpolations);
    void updateAggregationModel(const std::vector<std::string_view>& aggregations);
    void updateGridModel(const std::vector<std::string_view>& grids);

    void onConfigurationChange(const QString& configName);
    void onPollutantChange(const QString& configName);
    void onComputeSucceeded(const RasterPtr& raster);
    void onComputeFailed(const QString& message);

    void compute();
    void setInteractionEnabled(bool enabled);
    void populateColorMapCombo(bool invertGradients);

    Ui::MappingView _ui;
    QStandardItemModel _configurationModel;
    QStandardItemModel _interpolationModel;
    QStandardItemModel _pollutantModel;
    QStandardItemModel _aggregationModel;
    QStandardItemModel _gridModel;

    config::PollutantManager _pollutantMgr;
    PluginFactory _pluginFactory;
    Engine _engine;
    rio::config _config;
    std::unique_ptr<rio::obshandler> _dbq;
};
}
