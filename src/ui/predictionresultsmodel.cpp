#include "predictionresultsmodel.h"

#include "Station.h"
#include "data/ForecastBuffer.h"

namespace OPAQ
{

PredictionResultsModel::PredictionResultsModel(QObject* parent)
: QAbstractTableModel(parent)
, _buffer(nullptr)
, _rowCount(0)
, _colCount(0)
{
}

int PredictionResultsModel::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(_stations.size() * _forecastHorizonDays);
}

int PredictionResultsModel::columnCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(_headers.size());
}

QVariant PredictionResultsModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        int stationIndex = index.row() / _forecastHorizonDays;
        int forecastDay = (index.row() % _forecastHorizonDays);
        auto values = _buffer->getModelValues(_baseTime, TimeInterval(forecastDay, TimeInterval::Days), _stations.at(stationIndex)->getName(), _pollutantId, _aggregationType);

        auto value = values.at(index.column());
        if (std::fabs(value - _buffer->getNoData()) < std::numeric_limits<double>::epsilon())
        {
            return tr("No data");
        }

        return QString::number(value, 'f', 6);
    }

    return QVariant();
}

void PredictionResultsModel::updateResults(ForecastBuffer& buffer,
                                           DateTime baseTime,
                                           const std::vector<Station*>& stations,
                                           TimeInterval forecastHorizon,
                                           const std::string& pollutantId,
                                           Aggregation::Type agg)
{
    beginResetModel();

    _buffer              = &buffer;
    _baseTime            = baseTime;
    _stations            = stations;
    _forecastHorizon     = forecastHorizon;
    _forecastHorizonDays = static_cast<int>(forecastHorizon.getDays()) + 1;
    _pollutantId         = pollutantId;
    _aggregationType     = agg;
    _headers             = _buffer->getModelNames(pollutantId, agg);

    endResetModel();
}

QVariant PredictionResultsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal)
    {
        return QString(_headers.at(section).c_str());
    }
    else
    {
        int index       = section / _forecastHorizonDays;
        int forecastDay = (section % _forecastHorizonDays);
        assert(index < _stations.size());

        return QString("%1 Forecast %2").arg(_stations.at(index)->getName().c_str()).arg(forecastDay);
    }
}
}