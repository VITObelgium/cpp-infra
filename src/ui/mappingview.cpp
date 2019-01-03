#include "mappingview.h"

#include "AQNetwork.h"
#include "AQNetworkProvider.h"
#include "Station.h"
#include "data/ForecastBuffer.h"
#include "infra/cast.h"
#include "infra/log.h"
#include "uiutils.h"

#include <array>
#include <cassert>
#include <cinttypes>

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

    connect(_ui.browseButton, &QPushButton::clicked, this, [this]() {
        showConfigFileSelector();
    });

    connect(_ui.configPathCombo, QOverload<const QString&>::of(&QComboBox::activated), this, [this](const QString& path) {
        loadConfiguration(path);
    });

    connect(_ui.nameCombo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &MappingView::onConfigurationChange);
    connect(_ui.pollutantCombo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &MappingView::onPollutantChange);
    connect(_ui.interpolationCombo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &MappingView::onInterpolationChange);

    /*
    _aggregationModel.insertRows(0, static_cast<int>(s_aggregationTypes.size()));

    for (size_t i = 0; i < s_aggregationTypes.size(); ++i) {
        auto index = static_cast<int>(i);
        _aggregationModel.setItem(index, 0, new QStandardItem(QString(Aggregation::getDisplayName(s_aggregationTypes[index]).c_str())));
        _aggregationModel.setData(_aggregationModel.index(index, 0), s_aggregationTypes[index], Qt::UserRole);
    }*/

    _ui.nameCombo->setModel(&_configurationModel);
    _ui.pollutantCombo->setModel(&_pollutantModel);
    _ui.interpolationCombo->setModel(&_interpolationModel);
    _ui.aggregationCombo->setModel(&_aggregationModel);

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
        _config.set_configuration(configName.toStdString());
        _ui.configDescriptionLabel->setText(QString::fromStdString(_config.desc()));
        updatePollutantModel(_config.pol_names());
    } catch (const std::exception& e) {
        Log::error("Failed to change configuration: {}", e.what());
    }
}

void MappingView::onPollutantChange(const QString& name)
{
    _config.set_pol(name.toStdString());
    updateInterpolationModel(_config.ipol_names());
    updateAggregationModel(_config.aggr_names());
}

void MappingView::onInterpolationChange(const QString& name)
{
    _config.set_ipol(name.toStdString());
}

void MappingView::updateConfigurationsModel(const std::vector<std::string_view>& configurations)
{
    _configurationModel.clear();
    _configurationModel.insertRows(0, static_cast<int>(configurations.size()));

    int row = 0;
    for (auto& config : configurations) {
        _configurationModel.setItem(row++, 0, new QStandardItem(QString::fromUtf8(config.data(), truncate<int>(config.size()))));
    }
}

void MappingView::updatePollutantModel(const std::vector<std::string_view>& pollutants)
{
    _pollutantModel.clear();
    _pollutantModel.insertRows(0, static_cast<int>(pollutants.size()));

    int row = 0;
    for (auto& config : pollutants) {
        _pollutantModel.setItem(row++, 0, new QStandardItem(QString::fromUtf8(config.data(), truncate<int>(config.size()))));
    }
}

void MappingView::updateInterpolationModel(const std::vector<std::string_view>& interpollations)
{
    _interpolationModel.clear();
    _interpolationModel.insertRows(0, static_cast<int>(interpollations.size()));

    int row = 0;
    for (auto& config : interpollations) {
        _interpolationModel.setItem(row++, 0, new QStandardItem(QString::fromUtf8(config.data(), truncate<int>(config.size()))));
    }
}

void MappingView::updateAggregationModel(const std::vector<std::string_view>& aggregations)
{
    _aggregationModel.clear();
    _aggregationModel.insertRows(0, static_cast<int>(aggregations.size()));

    int row = 0;
    for (auto& aggr : aggregations) {
        if (aggr == "1h") {
            _aggregationModel.setItem(row++, 0, new QStandardItem(tr("Hourly")));
        } else if (aggr == "da") {
            _aggregationModel.setItem(row++, 0, new QStandardItem(tr("Daily")));
        } else if (aggr == "mo") {
            _aggregationModel.setItem(row++, 0, new QStandardItem(tr("Monthly")));
        } else {
            Log::warn("Unknown aggregation: {}", aggr);
        }
    }
}

}
