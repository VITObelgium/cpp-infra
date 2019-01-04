#include "mappingview.h"

#include "AQNetwork.h"
#include "AQNetworkProvider.h"
#include "Station.h"
#include "data/ForecastBuffer.h"
#include "infra/cast.h"
#include "infra/log.h"
#include "infra/string.h"
#include "modelrunner.hpp"
#include "uiinfra/userinteraction.h"
#include "uiutils.h"

#include <array>
#include <cassert>
#include <cinttypes>
#include <fmt/ostream.h>

#include <QFileDialog>
#include <QSettings>

namespace opaq {

using namespace inf;

static const int32_t s_maxRecentPaths                            = 5;
static const std::array<Aggregation::Type, 3> s_aggregationTypes = {
    Aggregation::DayAvg,
    Aggregation::Max1h,
    Aggregation::Max8h,
};

MappingView::MappingView(QWidget* parent)
: QWidget(parent)
, _engine(_pollutantMgr, _pluginFactory)
{
    _ui.setupUi(this);

    connect(_ui.browseButton, &QPushButton::clicked, this, &MappingView::showConfigFileSelector);
    connect(_ui.computeButton, &QPushButton::clicked, this, &MappingView::compute);

    connect(_ui.configPathCombo, QOverload<const QString&>::of(&QComboBox::activated), this, [this](const QString& path) {
        loadConfiguration(path);
    });

    connect(_ui.nameCombo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &MappingView::onConfigurationChange);
    connect(_ui.pollutantCombo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &MappingView::onPollutantChange);

    _ui.nameCombo->setModel(&_configurationModel);
    _ui.pollutantCombo->setModel(&_pollutantModel);
    _ui.interpolationCombo->setModel(&_interpolationModel);
    _ui.aggregationCombo->setModel(&_aggregationModel);
    _ui.gridCombo->setModel(&_gridModel);

    loadRecentConfigurations();

    if (!_ui.configPathCombo->currentText().isEmpty()) {
        loadConfiguration(_ui.configPathCombo->currentText());
    }
}

MappingView::~MappingView() = default;

void MappingView::showConfigFileSelector()
{
    auto filename = QFileDialog::getOpenFileName(this, tr("Load mapping configuration"), "", tr("Config Files (*.xml)"));
    if (filename.isEmpty()) {
        return;
    }

    loadConfiguration(filename);
    _ui.configPathCombo->setCurrentIndex(0);
}

void MappingView::loadConfiguration(const QString& path)
{
    try {
        QFileInfo fileInfo(path);
        // Change the working directory to the config file so the relative paths can be found
        QDir::setCurrent(fileInfo.absoluteDir().path());

        _config.parse_setup_file(path.toStdString());
        updateRecentConfiguration(path);
        updateConfigurationsModel(_config.configurations());
    } catch (const std::exception& e) {
        displayError(this, tr("Failed to load config file"), e.what());
    }
}

void MappingView::loadRecentConfigurations()
{
    QSettings settings;
    auto recentFilePaths = settings.value("RecentMappingConfigs").toStringList();
    _ui.configPathCombo->clear();
    _ui.configPathCombo->addItems(recentFilePaths);
}

void MappingView::updateRecentConfiguration(const QString& filePath)
{
    QSettings settings;
    QStringList recentFilePaths = settings.value("RecentMappingConfigs").toStringList();
    recentFilePaths.removeAll(filePath);
    recentFilePaths.prepend(filePath);
    while (recentFilePaths.size() > s_maxRecentPaths) {
        recentFilePaths.removeLast();
    }

    settings.setValue("RecentMappingConfigs", recentFilePaths);

    loadRecentConfigurations();
}

void MappingView::onConfigurationChange(const QString& configName)
{
    try {
        auto configString = configName.toStdString();
        auto desc         = _config.desc(configString);
        _ui.configDescriptionLabel->setText(QString::fromUtf8(desc.data(), truncate<int>(desc.size())));
        updatePollutantModel(_config.pol_names(configString));
        updateGridModel(_config.grid_names(configString));
    } catch (const std::exception& e) {
        Log::error("Failed to change configuration: {}", e.what());
    }
}

void MappingView::onPollutantChange(const QString& name)
{
    if (!name.isEmpty()) {
        auto configName    = _ui.nameCombo->currentText().toStdString();
        auto pollutantName = name.toStdString();

        updateInterpolationModel(_config.ipol_names(configName, pollutantName));
        updateAggregationModel(_config.aggr_names(configName, pollutantName));
    }
}

static void fillModel(QStandardItemModel& model, const std::vector<std::string_view>& names)
{
    model.clear();
    model.insertRows(0, static_cast<int>(names.size()));

    int row = 0;
    for (auto& name : names) {
        model.setItem(row++, 0, new QStandardItem(QString::fromUtf8(name.data(), truncate<int>(name.size()))));
    }
}

void MappingView::updateConfigurationsModel(const std::vector<std::string_view>& configurations)
{
    fillModel(_configurationModel, configurations);
    _ui.nameCombo->setCurrentIndex(0);
}

void MappingView::updatePollutantModel(const std::vector<std::string_view>& pollutants)
{
    fillModel(_pollutantModel, pollutants);
    _ui.pollutantCombo->setCurrentIndex(0);
}

void MappingView::updateInterpolationModel(const std::vector<std::string_view>& interpollations)
{
    fillModel(_interpolationModel, interpollations);
    _ui.interpolationCombo->setCurrentIndex(0);
}

void MappingView::updateAggregationModel(const std::vector<std::string_view>& aggregations)
{
    _aggregationModel.clear();
    _aggregationModel.insertRows(0, static_cast<int>(aggregations.size()));

    int row = 0;
    for (auto& aggr : aggregations) {
        QStandardItem* item = new QStandardItem();

        if (aggr == "1h") {
            item->setText(tr("Hourly"));
        } else if (aggr == "da") {
            item->setText(tr("Daily"));
        } else if (aggr == "mo") {
            item->setText(tr("Monthly"));
        }

        if (item) {
            item->setData(QString::fromUtf8(aggr.data(), truncate<int>(aggr.size())), Qt::UserRole);
            _aggregationModel.setItem(row++, 0, item);
        } else {
            Log::warn("Unknown aggregation: {}", aggr);
        }
    }

    _ui.aggregationCombo->setCurrentIndex(0);
}

void MappingView::updateGridModel(const std::vector<std::string_view>& grids)
{
    fillModel(_gridModel, grids);
    _ui.gridCombo->setCurrentIndex(0);
}

void MappingView::compute()
{
    try {
        _config.apply_config(
            _ui.nameCombo->currentText().toStdString(),
            _ui.pollutantCombo->currentText().toStdString(),
            _ui.interpolationCombo->currentText().toStdString(),
            _ui.aggregationCombo->currentData(Qt::UserRole).toString().toStdString(),
            _ui.gridCombo->currentText().toStdString());
        rio::run_model(_config);
    } catch (const std::exception& e) {
        Log::error("Compute failed: {}", e.what());
        uiinfra::displayError(tr("Mapping failed"), QString::fromUtf8(e.what()));
    }
}

}
