#include "stationresultsmodel.h"

#include "Station.h"
#include "data/ForecastBuffer.h"

#include <QColor>

namespace opaq
{

StationResultsModel::StationResultsModel(QObject* parent)
: QAbstractTableModel(parent)
, _buffer(nullptr)
, _rowCount(0)
, _colCount(0)
{
}

int StationResultsModel::rowCount(const QModelIndex& /*parent*/) const
{
    return static_cast<int>(_forecastHorizon.count());
}

int StationResultsModel::columnCount(const QModelIndex& /*parent*/) const
{
    if (_headers.size() == 0)
    {
        return 0;
    }

    return static_cast<int>(_headers.size()) + 1;
}

QVariant StationResultsModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (index.row() == 0)
        {
            return QString::number(index.column());
        }

        try
        {
            int forecastDay = index.column();
            int modelIndex  = index.row() - 1;
            auto values     = _buffer->getModelValues(_baseTime, chrono::days(forecastDay), _stationName, _pollutantId, _aggregationType);

            auto value = values.at(modelIndex);
            if (std::fabs(value - _buffer->getNoData()) < std::numeric_limits<double>::epsilon())
            {
                return tr("No data");
            }

            return QString::number(value, 'f', 6);
        }
        catch (const std::exception&)
        {
            return tr("No data");
        }
    }
    else if (role == Qt::BackgroundRole)
    {
        for (auto& rect : _mapping)
        {
            if (rect.contains(index.column(), index.row()))
            {
                return QColor(_mapping.key(rect));
            }
        }

        // cell not mapped return white color
        return QColor(Qt::white);
    }

    return QVariant();
}

void StationResultsModel::updateResults(ForecastBuffer& buffer,
                                        chrono::date_time baseTime,
                                        const std::string& stationName,
                                        chrono::days forecastHorizon,
                                        const std::string& pollutantId,
                                        Aggregation::Type agg)
{
    beginResetModel();

    _buffer              = &buffer;
    _baseTime            = baseTime;
    _stationName         = stationName;
    _forecastHorizon     = forecastHorizon;
    _pollutantId         = pollutantId;
    _aggregationType     = agg;
    _headers             = _buffer->getModelNames(pollutantId, agg);

    endResetModel();
}

QVariant StationResultsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Vertical)
    {
        if (section == 0 || _headers.empty())
        {
            return QVariant();
        }

        return QString(_headers.at(section - 1).c_str());
    }
    else
    {
        return QString("Day+%1").arg(section);
    }
}

void StationResultsModel::addMapping(QString color, QRect area)
{
    _mapping.insertMulti(color, area);
}

}