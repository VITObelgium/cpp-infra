#include "opaqviewer.h"

#include "Engine.h"
#include "AQNetwork.h"
#include "AQNetworkProvider.h"
#include "ConfigurationHandler.h"
#include "uiutils.h"
#include "tools/DateTimeTools.h"

#include <cassert>

Q_DECLARE_METATYPE(OPAQ::Aggregation::Type)

namespace OPAQ
{

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

void OpaqViewer::setStationModel(QAbstractItemModel& model)
{
    _ui.stationComboBox->setModel(&model);
    
    connect(&model, &QAbstractListModel::dataChanged, [this] () {
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

void OpaqViewer::setForecastHorizon(const TimeInterval& fcHor)
{
    _forecastHorizon = fcHor;
    _ui.resultsView->setForecastHorizon(fcHor);
}

void OpaqViewer::updateResultsForCurrentStation()
{
    if (!_buffer)
    {
        // no simulation has run yet
        return;
    }

    _model.updateResults(*_buffer, basetime(), station(), _forecastHorizon, pollutant(), aggregation());
    _ui.tableView->setRowHidden(0, true);
}

void OpaqViewer::setModels(const std::vector<Config::Component*>& models)
{
    _ui.resultsView->setModels(_model, models);
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

std::string OpaqViewer::basetime() const noexcept
{
    return _ui.basetimeDateEdit->date().toString("yyyy-MM-dd").toStdString();
}

void OpaqViewer::runSimulation()
{
    assert(_config);
    assert(_engine);

    auto pol = pollutant();
    _config->getOpaqRun().setPollutantName(pollutant(), Aggregation::getName(aggregation()));

    auto baset = basetime();
    if (!baset.empty())
    {
        auto& basetimes = _config->getOpaqRun().getBaseTimes();
        basetimes.clear();

        try
        {
            auto baseTime = OPAQ::DateTimeTools::parseDate(baset);
            int dayCount = 1;
            for (int i = 0; i < dayCount; ++i)
            {
                basetimes.push_back(baseTime);
                baseTime.addDays(1);
            }
        }
        catch (const OPAQ::ParseException&)
        {
            displayError(this, tr("Failed to parse base time"), QString("Failed to parse base time : %1").arg(baset.c_str()));
            return;
        }
    }

    try
    {
        _engine->run(_config->getOpaqRun());

        auto& aqNetworkProvider = _engine->componentManager().getComponent<AQNetworkProvider>(_config->getOpaqRun().getNetworkProvider()->name);
        auto& buffer = _engine->componentManager().getComponent<ForecastBuffer>(_config->getOpaqRun().getForecastStage()->getBuffer().name);
        auto forecastHorizon = _config->getOpaqRun().getForecastStage()->getHorizon();

        updateResultsForCurrentStation();
    }
    catch (const std::exception& e)
    {
        displayError(this, tr("Failed to run simulation"), e.what());
    }
}

}
