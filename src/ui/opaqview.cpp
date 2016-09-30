#include "opaqview.h"
#include "ui_opaqview.h"

#include "Station.h"

#include <cassert>

namespace OPAQ
{

OpaqView::OpaqView(QWidget* parent)
: QWidget(parent)
, _ui(std::make_unique<Ui::OpaqView>())
, _buffer(nullptr)
{
    _ui->setupUi(this);

    connect(_ui->stationCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this] (int) {
        updateResultsForCurrentStation();
    });
}

OpaqView::~OpaqView() = default;

void OpaqView::setStations(const std::vector<Station*>& stations)
{
    for (auto* station : stations)
    {
        auto displayName = QString("%1 (%2)").arg(station->getDescription().c_str()).arg(station->getName().c_str());
        _ui->stationCombo->addItem(displayName, QString(station->getName().c_str()));
    }
}

void OpaqView::setModels(const std::vector<Config::Component*>& models)
{
    _ui->resultsView->setModels(models);
}

void OpaqView::setBaseTime(const DateTime& baseTime)
{
    _baseTime = baseTime;
}

void OpaqView::setForecastHorizon(const TimeInterval& forecastHorizon)
{
    _forecastHorizon = forecastHorizon;
    _ui->resultsView->setForecastHorizon(forecastHorizon);
}

void OpaqView::setForecastBuffer(ForecastBuffer& buffer)
{
    _buffer = &buffer;
}

void OpaqView::setPollutantId(const std::string& pollutantId)
{
    _pollutantId = pollutantId;
}

void OpaqView::setAggregationType(Aggregation::Type agg)
{
    _aggregation = agg;
}

void OpaqView::updateResultsForCurrentStation()
{
    if (!_buffer)
    {
        // no simulation has run yet
        return;
    }

    auto station = _ui->stationCombo->currentData().value<QString>().toStdString();
    _ui->resultsView->updateResultsForStation(*_buffer, _baseTime, station, _forecastHorizon, _pollutantId, _aggregation);
}

}