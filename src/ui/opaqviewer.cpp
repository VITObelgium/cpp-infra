#include "opaqviewer.h"

#include "AQNetwork.h"
#include "AQNetworkProvider.h"
#include "ConfigurationHandler.h"
#include "Engine.h"

#include "uiinfra/userinteraction.h"

#include <cassert>

Q_DECLARE_METATYPE(opaq::Aggregation::Type)

namespace opaq {

using namespace inf;
using namespace chrono_literals;

OpaqViewer::OpaqViewer(QWidget* parent)
: QWidget(parent)
, _model(this)
, _buffer(nullptr)
, _config(nullptr)
, _engine(nullptr)
{
    _ui.setupUi(this);

    _ui.tableView->setModel(&_model);
    _ui.tableView->setRowHidden(0, true);

    _ui.basetimeDateEdit->setDate(QDate::currentDate());

    connect(_ui.stationComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int) {
        updateResultsForCurrentStation();
    });

    connect(_ui.simulateButton, &QPushButton::clicked, [this]() {
        runSimulation();
    });
}

void OpaqViewer::setConfig(ConfigurationHandler& config)
{
    _config = &config;
}

void OpaqViewer::setEngine(Engine& engine)
{
    _engine = &engine;
}

void OpaqViewer::setModels(const std::vector<config::Component>& models)
{
    _ui.resultsView->setModels(_model, models);
}

void OpaqViewer::setStationModel(QAbstractItemModel& model)
{
    _ui.stationComboBox->setModel(&model);

    connect(&model, &QAbstractListModel::dataChanged, [this]() {
        _ui.stationComboBox->setCurrentIndex(0);
    });
}

void OpaqViewer::setPollutantModel(QAbstractItemModel& model)
{
    _ui.pollutantComboBox->setModel(&model);

    connect(&model, &QAbstractListModel::dataChanged, [this]() {
        _ui.pollutantComboBox->setCurrentIndex(0);
    });
}

void OpaqViewer::setAggregationModel(QAbstractItemModel& model)
{
    _ui.aggregationComboBox->setModel(&model);
}

void OpaqViewer::setForecastBuffer(ForecastBuffer& buffer)
{
    _buffer = &buffer;
}

void OpaqViewer::resetForecastBuffer()
{
    _buffer = nullptr;
}

void OpaqViewer::setForecastHorizon(chrono::days fcHor)
{
    _forecastHorizon = fcHor;
    _ui.resultsView->setForecastHorizon(fcHor);
}

void OpaqViewer::updateResultsForCurrentStation()
{
    if (!_buffer) {
        // no simulation has run yet
        return;
    }

    _model.updateResults(*_buffer, basetime(), station(), _forecastHorizon, pollutant(), aggregation());
    _ui.tableView->setRowHidden(0, true);

    for (int i = 0; i <= static_cast<int>(_forecastHorizon.count()); ++i) {
        _ui.tableView->setColumnWidth(i, 60);
    }
}

std::string OpaqViewer::pollutant() const noexcept
{
    return _ui.pollutantComboBox->currentData(Qt::UserRole).value<QString>().toStdString();
}

Aggregation::Type OpaqViewer::aggregation() const noexcept
{
    assert(_ui.aggregationComboBox->currentData(Qt::UserRole).canConvert<Aggregation::Type>());
    return _ui.aggregationComboBox->currentData(Qt::UserRole).value<Aggregation::Type>();
}

std::string OpaqViewer::station() const noexcept
{
    return _ui.stationComboBox->currentData(Qt::UserRole).value<QString>().toStdString();
}

chrono::date_time OpaqViewer::basetime() const noexcept
{
    return chrono::from_date_string(_ui.basetimeDateEdit->date().toString("yyyy-MM-dd").toStdString());
}

void OpaqViewer::runSimulation()
{
    assert(_config);
    assert(_engine);

    auto pol = pollutant();
    _config->getOpaqRun().setPollutantName(pollutant(), Aggregation::getName(aggregation()));

    auto baseTime = basetime();
    if (baseTime != chrono::date_time()) {
        _config->getOpaqRun().clearBaseTimes();

        int dayCount = 1;
        for (int i = 0; i < dayCount; ++i) {
            _config->getOpaqRun().addBaseTime(baseTime);
            baseTime += 1_d;
        }
    }

    try {
        _engine->run(_config->getOpaqRun());
        updateResultsForCurrentStation();
    } catch (const std::exception& e) {
        ui::displayError(tr("Failed to run simulation"), e.what());
    }
}

}
