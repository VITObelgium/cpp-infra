#include "mappingview.h"

#include "AQNetwork.h"
#include "AQNetworkProvider.h"
#include "Station.h"
#include "data/ForecastBuffer.h"
#include "gridmapper.hpp"
#include "infra/cast.h"
#include "infra/log.h"
#include "infra/string.h"
#include "jobrunner.h"
#include "memorywriter.hpp"
#include "modelrunner.hpp"
#include "typeregistrations.h"
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
using namespace std::string_literals;

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
    connect(this, &MappingView::computeProgress, _ui.progressBar, &QProgressBar::setValue);
    connect(this, &MappingView::computeSucceeded, this, &MappingView::onComputeSucceeded);
    connect(this, &MappingView::computeFailed, this, &MappingView::onComputeFailed);

    _ui.nameCombo->setModel(&_configurationModel);
    _ui.pollutantCombo->setModel(&_pollutantModel);
    _ui.interpolationCombo->setModel(&_interpolationModel);
    _ui.aggregationCombo->setModel(&_aggregationModel);
    _ui.gridCombo->setModel(&_gridModel);

    loadRecentConfigurations();

    if (!_ui.configPathCombo->currentText().isEmpty()) {
        loadConfiguration(_ui.configPathCombo->currentText());
    }

    _ui.progressBar->hide();

    qRegisterMetaType<RasterPtr>("RasterPtr");
    qRegisterMetaType<RasterDataId>("RasterDataId");
    qRegisterMetaType<Legend>("inf::Legend");
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

void MappingView::onComputeSucceeded(const RasterPtr& raster)
{
    _ui.progressBar->hide();
    _ui.computeButton->setText(tr("Compute"));
    _ui.map->setData(raster);
    setInteractionEnabled(true);
}

void MappingView::onComputeFailed(const QString& message)
{
    _ui.progressBar->hide();
    uiinfra::displayError(tr("Mapping failed"), message);
    _ui.computeButton->setText(tr("Compute"));
    setInteractionEnabled(true);
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

static inf::GeoMetadata create_metadata(double xul, double yul, double cellSize, int32_t rows, int32_t cols, int32_t epsg, std::optional<double> nodata)
{
    inf::GeoMetadata meta;
    meta.xll      = xul;
    meta.yll      = yul - (rows * cellSize);
    meta.cellSize = cellSize;
    meta.rows     = rows;
    meta.cols     = cols;
    meta.nodata   = nodata;
    meta.set_projection_from_epsg(epsg);
    return meta;
}

void MappingView::compute()
{
    if (_ui.progressBar->isVisible()) {
        // Calculation in progress
        _ui.computeButton->setEnabled(false);
        _cancel = true;
        return;
    }

    std::string configName     = _ui.nameCombo->currentText().toStdString();
    std::string pollutant      = _ui.pollutantCombo->currentText().toStdString();
    std::string interpollation = _ui.interpolationCombo->currentText().toStdString();
    std::string aggregation    = _ui.aggregationCombo->currentData(Qt::UserRole).toString().toStdString();
    std::string grid           = _ui.gridCombo->currentText().toStdString();
    std::string startDate      = _ui.startDate->dateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")).toStdString();
    std::string endDate;
    if (_ui.endDateCheck->isChecked()) {
        endDate = _ui.endDate->dateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")).toStdString();
    }

    static std::unordered_map<std::string, rio::griddefinition> s_definitionLookup{{
        {"4x4"s, {create_metadata(22000, 248000, 4000, 57, 69, 31370, -9999), "%base%/grids/rio_4x4_grid.map"}},
        {"4x4e1"s, {create_metadata(18000, 252000, 4000, 59, 71, 31370, -9999), "%base%/grids/rio_4x4e1_grid.map"}},
        {"4x4e2"s, {create_metadata(14000, 256000, 4000, 61, 73, 31370, -9999), "%base%/grids/rio_4x4e2_grid.map"}},
    }};

    if (s_definitionLookup.count(grid) == 0) {
        uiinfra::displayError(tr("Missing grid definition"), tr("No grid definition available for %1 grid").arg(grid.c_str()));
        return;
    }

    setInteractionEnabled(false);

    _cancel = false;
    _ui.progressBar->setValue(0);
    _ui.progressBar->show();
    _ui.computeButton->setText(tr("Cancel"));

    _ui.progressBar->setValue(0);

    JobRunner::queue([=]() {
        try {
            _config.apply_config(
                configName,
                pollutant,
                interpollation,
                aggregation,
                grid,
                startDate,
                endDate);

            rio::output output(std::make_unique<rio::memorywriter>(s_definitionLookup.at(grid)));
            rio::run_model(_config, output, [this](int progress) { emit computeProgress(progress);  return !_cancel; });

            RasterDisplayData displayData;
            auto memWriter = dynamic_cast<rio::memorywriter*>(output.list().front().get());
            auto raster    = std::make_shared<gdx::DenseRaster<double>>(memWriter->metadata(), memWriter->data());
            emit computeSucceeded(raster);
        } catch (const std::exception& e) {
            emit computeFailed(QString::fromStdString(e.what()));
        }
    });
}

void MappingView::setInteractionEnabled(bool enabled)
{
    _ui.configPathCombo->setEnabled(enabled);
    _ui.browseButton->setEnabled(enabled);
    _ui.nameCombo->setEnabled(enabled);
    _ui.pollutantCombo->setEnabled(enabled);
    _ui.gridCombo->setEnabled(enabled);
    _ui.interpolationCombo->setEnabled(enabled);
    _ui.aggregationCombo->setEnabled(enabled);
    _ui.startDate->setEnabled(enabled);
    _ui.endDateCheck->setEnabled(enabled);
    _ui.endDate->setEnabled(enabled && _ui.endDateCheck->isChecked());
}

}
