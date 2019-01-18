#include "opaqview.h"

#include "AQNetwork.h"
#include "AQNetworkProvider.h"
#include "Station.h"
#include "data/ForecastBuffer.h"
#include "uiinfra/userinteraction.h"

#include <array>
#include <cassert>
#include <cinttypes>

#include <QFileDialog>
#include <QSettings>

Q_DECLARE_METATYPE(opaq::Aggregation::Type)

namespace opaq {

using namespace inf;

static const std::array<Aggregation::Type, 3> s_aggregationTypes = {
    Aggregation::DayAvg,
    Aggregation::Max1h,
    Aggregation::Max8h};

OpaqView::OpaqView(QWidget* parent)
: QWidget(parent)
, _engine(_pollutantMgr, _pluginFactory)
{
    _ui.setupUi(this);

    connect(_ui.configPath, &FileSelectionComboBox::pathChanged, this, &OpaqView::loadConfiguration);

    _aggregationModel.insertRows(0, static_cast<int>(s_aggregationTypes.size()));

    for (size_t i = 0; i < s_aggregationTypes.size(); ++i) {
        auto index = static_cast<int>(i);
        _aggregationModel.setItem(index, 0, new QStandardItem(QString(Aggregation::getDisplayName(s_aggregationTypes[index]).c_str())));
        _aggregationModel.setData(_aggregationModel.index(index, 0), s_aggregationTypes[index], Qt::UserRole);
    }

    _ui.configPath->setFileSelectorFilter(tr("Config files (*.xml)"));
    _ui.configPath->setConfigName("RecentForecastConfigs");

    _ui.viewerTab->setConfig(_config);
    _ui.viewerTab->setEngine(_engine);
    _ui.viewerTab->setStationModel(_stationModel);
    _ui.viewerTab->setPollutantModel(_pollutantModel);
    _ui.viewerTab->setAggregationModel(_aggregationModel);

    _ui.validationTab->setConfig(_config);
    _ui.validationTab->setEngine(_engine);
    _ui.validationTab->setStationModel(_stationModel);
    _ui.validationTab->setPollutantModel(_pollutantModel);
    _ui.validationTab->setAggregationModel(_aggregationModel);

    _ui.tabWidget->setDisabled(true);
}

OpaqView::~OpaqView() = default;

void OpaqView::setModels(const std::vector<config::Component>& models)
{
    _ui.viewerTab->setModels(models);
    _ui.validationTab->setModels(models);
}

void OpaqView::loadConfiguration(const QString& path)
{
    try {
        _ui.viewerTab->resetForecastBuffer();

        QFileInfo fileInfo(path);
        // Change the working directory to the config file so the relative paths can be found
        QDir::setCurrent(fileInfo.absoluteDir().path());

        _config.parseConfigurationFile(path.toStdString(), _pollutantMgr);
        _engine.prepareRun(_config.getOpaqRun());
        _config.validateConfiguration(_pollutantMgr);

        auto& aqNetworkProvider = _engine.componentManager().getComponent<AQNetworkProvider>(_config.getOpaqRun().getNetworkProvider()->name);
        auto& buffer            = _engine.componentManager().getComponent<ForecastBuffer>(_config.getOpaqRun().getForecastStage()->getBuffer().name);
        auto forecastHorizon    = _config.getOpaqRun().getForecastStage()->getHorizon();

        updateStationModel(aqNetworkProvider.getAQNetwork().getStations());

        _ui.viewerTab->setForecastBuffer(buffer);
        _ui.viewerTab->setForecastHorizon(forecastHorizon);

        updatePollutantModel();

        auto fcStage = _config.getOpaqRun().getForecastStage();
        if (fcStage) {
            setModels(fcStage->getModels());
        }

        _ui.tabWidget->setDisabled(false);
    } catch (const std::exception& e) {
        ui::displayError(tr("Failed to load config file"), e.what());
    }
}

void OpaqView::updateStationModel(const std::vector<Station>& stations)
{
    _stationModel.clear();
    _stationModel.insertRows(0, static_cast<int>(stations.size()));

    int row = 0;
    for (auto& station : stations) {
        _stationModel.setItem(row, 0, new QStandardItem(QString("%1 [%2]").arg(station.getDescription().c_str()).arg(station.getName().c_str())));
        _stationModel.setData(_stationModel.index(row++, 0), QString(station.getName().c_str()), Qt::UserRole);
    }
}

void OpaqView::updatePollutantModel()
{
    auto& pollutants = _pollutantMgr.getList();
    _pollutantModel.clear();
    _pollutantModel.insertRows(0, static_cast<int>(pollutants.size()));

    int row = 0;
    for (auto& pol : pollutants) {
        _pollutantModel.setItem(row, 0, new QStandardItem(QString(pol.getDescription().c_str())));
        _pollutantModel.setData(_pollutantModel.index(row++, 0), QString(pol.getName().c_str()), Qt::UserRole);
    }
}

}
