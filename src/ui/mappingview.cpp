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

#include <array>
#include <cassert>
#include <cinttypes>
#include <fmt/ostream.h>

#include <QFileDialog>
#include <QSettings>
#include <qstringlistmodel.h>

namespace opaq {

using namespace inf;
using namespace std::string_literals;

static const int32_t s_maxRecentPaths                            = 5;

MappingView::MappingView(QWidget* parent)
: QWidget(parent)
, _engine(_pollutantMgr, _pluginFactory)
{
    _ui.setupUi(this);

    connect(_ui.browseConfigButton, &QPushButton::clicked, this, &MappingView::showConfigFileSelector);
    connect(_ui.browseForecastButton, &QPushButton::clicked, this, &MappingView::showForecastFileSelector);
    connect(_ui.configPathCombo, QOverload<const QString&>::of(&QComboBox::activated), this, [this](const QString& path) {
        loadConfiguration(path);
    });

    connect(_ui.nameCombo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &MappingView::onConfigurationChange);
    connect(_ui.pollutantCombo, QOverload<const QString&>::of(&QComboBox::currentIndexChanged), this, &MappingView::onPollutantChange);
    connect(_ui.invertColorMapCheck, &QCheckBox::stateChanged, this, &MappingView::populateColorMapCombo);

    connect(_ui.colorMapCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        _ui.map->setColorMap(_ui.colorMapCombo->currentText().toStdString());
    });

    connect(_ui.gridCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MappingView::compute);
    connect(_ui.aggregationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MappingView::compute);
    connect(_ui.interpolationCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MappingView::compute);
    connect(_ui.pollutantCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MappingView::compute);
    connect(_ui.startDate, &QDateTimeEdit::dateTimeChanged, this, &MappingView::compute);

    connect(this, &MappingView::computeSucceeded, this, &MappingView::onComputeSucceeded);
    connect(this, &MappingView::computeFailed, this, &MappingView::onComputeFailed);

    _ui.nameCombo->setModel(&_configurationModel);
    _ui.pollutantCombo->setModel(&_pollutantModel);
    _ui.interpolationCombo->setModel(&_interpolationModel);
    _ui.aggregationCombo->setModel(&_aggregationModel);
    _ui.gridCombo->setModel(&_gridModel);

    loadRecentConfigurations();
    loadRecentForecasts();

    populateColorMapCombo(_ui.invertColorMapCheck->isChecked());

    if (!_ui.configPathCombo->currentText().isEmpty()) {
        loadConfiguration(_ui.configPathCombo->currentText());
    }

    if (!_ui.forecastPathCombo->currentText().isEmpty()) {
        try {
            _dbq = std::make_unique<rio::dbqfile>(_ui.forecastPathCombo->currentText().toStdString(), "RIO"s);
        } catch (const std::exception& e) {
            Log::error("Failed to load forecast data: {}", e.what());
        }
    }

    qRegisterMetaType<RasterPtr>("RasterPtr");
    qRegisterMetaType<RasterDataId>("RasterDataId");
    qRegisterMetaType<Legend>("inf::Legend");
}

MappingView::~MappingView() = default;

void MappingView::applyLegendSettings(const LegendSettings& settings)
{
    _ui.map->applyLegendSettings(settings);
    compute();
}

void MappingView::showConfigFileSelector()
{
    auto filename = QFileDialog::getOpenFileName(this, tr("Load mapping configuration"), "", tr("Config files (*.xml)"));
    if (filename.isEmpty()) {
        return;
    }

    loadConfiguration(filename);
    _ui.configPathCombo->setCurrentIndex(0);
}

void MappingView::showForecastFileSelector()
{
    auto filename = QFileDialog::getOpenFileName(this, tr("Load forecast data"), "", tr("Forecast files (*.dat *.txt *.csv)"));
    if (filename.isEmpty()) {
        return;
    }

    try {
        _dbq = std::make_unique<rio::dbqfile>(filename.toStdString(), "RIO"s);
        updateRecentForecasts(filename);
        _ui.forecastPathCombo->setCurrentIndex(0);
    } catch (const std::exception& e) {
        ui::displayError(tr("Failed to load forecast data"), e.what());
    }
}

void MappingView::loadConfiguration(const QString& path)
{
    try {
        QFileInfo fileInfo(path);
        // Change the working directory to the config file so the relative paths can be found
        QDir::setCurrent(fileInfo.absoluteDir().path());

        _config.parse_setup_file(path.toStdString());
        updateRecentConfigurations(path);
        updateConfigurationsModel(_config.configurations());
    } catch (const std::exception& e) {
        ui::displayError(tr("Failed to load config file"), e.what());
    }
}

void MappingView::loadRecentConfigurations()
{
    QSettings settings;
    auto recentFilePaths = settings.value("RecentMappingConfigs").toStringList();
    _ui.configPathCombo->clear();
    _ui.configPathCombo->addItems(recentFilePaths);
}

void MappingView::loadRecentForecasts()
{
    QSettings settings;
    auto recentFilePaths = settings.value("RecentForecasts").toStringList();
    _ui.forecastPathCombo->clear();
    _ui.forecastPathCombo->addItems(recentFilePaths);
}

void MappingView::updateRecentConfigurations(const QString& filePath)
{
    updateRecentpathsList("RecentMappingConfigs", filePath);
    loadRecentConfigurations();
}

void MappingView::updateRecentForecasts(const QString& filePath)
{
    updateRecentpathsList("RecentForecasts", filePath);
    loadRecentForecasts();
}

void MappingView::updateRecentpathsList(const QString& settingsName, const QString& filePath)
{
    QSettings settings;
    QStringList recentFilePaths = settings.value(settingsName).toStringList();
    recentFilePaths.removeAll(filePath);
    recentFilePaths.prepend(filePath);
    while (recentFilePaths.size() > s_maxRecentPaths) {
        recentFilePaths.removeLast();
    }

    settings.setValue(settingsName, recentFilePaths);
}

void MappingView::onConfigurationChange(const QString& configName)
{
    try {
        QSignalBlocker blocker1(_ui.pollutantCombo), blocker2(_ui.interpolationCombo), blocker3(_ui.aggregationCombo), blocker4(_ui.gridCombo);

        auto configString = configName.toStdString();
        auto desc         = _config.desc(configString);
        _ui.configDescriptionLabel->setText(QString::fromUtf8(desc.data(), truncate<int>(desc.size())));
        updatePollutantModel(_config.pol_names(configString));
        updateGridModel(_config.grid_names(configString));

        auto pollutant = _ui.pollutantCombo->currentText().toStdString();
        updateInterpolationModel(_config.ipol_names(configString, pollutant));
        updateAggregationModel(_config.aggr_names(configString, pollutant));

        compute();
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
    _ui.map->setData(raster);
    setInteractionEnabled(true);
}

void MappingView::onComputeFailed(const QString& message)
{
    _ui.map->clearData();
    Log::error("Mapping failed {}", message.toStdString());
    //ui::displayError(tr("Mapping failed"), message);
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
        } else if (aggr == "wk") {
            item->setText(tr("Weekly"));
        } else if (aggr == "m1") {
            item->setText(tr("Max 1h"));
        } else if (aggr == "m1") {
            item->setText(tr("Max 8h"));
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
    std::string configName     = _ui.nameCombo->currentText().toStdString();
    std::string pollutant      = _ui.pollutantCombo->currentText().toStdString();
    std::string interpollation = _ui.interpolationCombo->currentText().toStdString();
    std::string aggregation    = _ui.aggregationCombo->currentData(Qt::UserRole).toString().toStdString();
    std::string grid           = _ui.gridCombo->currentText().toStdString();
    std::string startDate      = _ui.startDate->dateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")).toStdString();
    std::string forecastData   = _ui.forecastPathCombo->currentText().toStdString();
    std::string endDate;

    static std::unordered_map<std::string, rio::griddefinition> s_definitionLookup{{
        {"4x4"s, {create_metadata(22000, 248000, 4000, 57, 69, 31370, -9999), "%base%/grids/rio_4x4_grid.map"}},
        {"4x4e1"s, {create_metadata(18000, 252000, 4000, 59, 71, 31370, -9999), "%base%/grids/rio_4x4e1_grid.map"}},
        {"4x4e2"s, {create_metadata(14000, 256000, 4000, 61, 73, 31370, -9999), "%base%/grids/rio_4x4e2_grid.map"}},
    }};

    setInteractionEnabled(false);

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

            if (s_definitionLookup.count(grid) == 0) {
                throw RuntimeError("No grid definition available for {} grid", grid);
            }

            if (!_dbq) {
                throw RuntimeError("No valid forecast data loaded");
            }

            rio::output output(std::make_unique<rio::memorywriter>(s_definitionLookup.at(grid)));
            rio::modelcomponents rioComponents;
            rioComponents.obsHandler    = _dbq.get();
            rioComponents.outputHandler = &output;

            rio::run_model(_config, rioComponents);

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
    /*_ui.configPathCombo->setEnabled(enabled);
    _ui.browseConfigButton->setEnabled(enabled);
    _ui.browseForecastButton->setEnabled(enabled);
    _ui.nameCombo->setEnabled(enabled);
    _ui.pollutantCombo->setEnabled(enabled);
    _ui.gridCombo->setEnabled(enabled);
    _ui.interpolationCombo->setEnabled(enabled);
    _ui.aggregationCombo->setEnabled(enabled);
    _ui.colorMapCombo->setEnabled(enabled);
    _ui.invertColorMapCheck->setEnabled(enabled);*/

    // use readonly instead of enabled, otherwise focus jumps to next date section
    _ui.startDate->setReadOnly(!enabled);
}

void MappingView::populateColorMapCombo(bool invertGradients)
{
    //https://matplotlib.org/examples/color/colormaps_reference.html
    static QStringList colorMaps = {
        // diverging
        QStringLiteral("rdylgn"),
        QStringLiteral("rdylbu"),
        QStringLiteral("piyg"),
        QStringLiteral("prgn"),
        QStringLiteral("brbg"),
        QStringLiteral("puor"),
        QStringLiteral("rdgy"),
        QStringLiteral("rdbu"),
        QStringLiteral("spectral"),
        // sequential
        QStringLiteral("greys"),
        QStringLiteral("purples"),
        QStringLiteral("blues"),
        QStringLiteral("greens"),
        QStringLiteral("oranges"),
        QStringLiteral("reds"),
        QStringLiteral("ylorbr"),
        QStringLiteral("ylorrd"),
        QStringLiteral("orrd"),
        QStringLiteral("purd"),
        QStringLiteral("rdpu"),
        QStringLiteral("bupu"),
        QStringLiteral("gnbu"),
        QStringLiteral("pubu"),
        QStringLiteral("ylgnbu"),
        QStringLiteral("pubugn"),
        QStringLiteral("bugn"),
        QStringLiteral("ylgn"),
        // sequential 2
        QStringLiteral("gray"),
        QStringLiteral("bone"),
        QStringLiteral("pink"),
        QStringLiteral("spring"),
        QStringLiteral("summer"),
        QStringLiteral("autumn"),
        QStringLiteral("winter"),
        QStringLiteral("cool"),
        QStringLiteral("hot"),
        QStringLiteral("copper"),
        // miscellaneous
        QStringLiteral("gist_earth"),
        QStringLiteral("terrain"),
        QStringLiteral("gist_stern"),
        QStringLiteral("hsv"),
        QStringLiteral("jet"),
        QStringLiteral("spectral"),
        QStringLiteral("gist_ncar"),
    };

    auto currentIndex = _ui.colorMapCombo->currentIndex();

    auto cmaps = colorMaps;
    if (invertGradients) {
        std::transform(cmaps.begin(), cmaps.end(), cmaps.begin(), [](const QString& cmap) {
            return cmap + "_r";
        });
    }

    _ui.colorMapCombo->setModel(new QStringListModel(cmaps, this));
    if (currentIndex >= 0) {
        _ui.colorMapCombo->setCurrentIndex(currentIndex);
    }
}
}
