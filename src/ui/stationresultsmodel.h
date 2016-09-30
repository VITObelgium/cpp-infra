#pragma once

#include "Aggregation.h"
#include "TimeInterval.h"

#include <QAbstractTableModel>
#include <QRect>

namespace OPAQ
{

class ForecastBuffer;
class Station;

class StationResultsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    StationResultsModel(QObject* parent);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void updateResults(ForecastBuffer& buffer,
                       DateTime baseTime,
                       const std::string& stationName,
                       TimeInterval forecastHorizonDays,
                       const std::string& pollutantId,
                       Aggregation::Type agg);

    void addMapping(QString color, QRect area);

private:
    ForecastBuffer* _buffer;
    int _rowCount;
    int _colCount;
    int _forecastHorizonDays;
    DateTime _baseTime;
    TimeInterval _forecastHorizon;
    std::string _stationName;
    std::string _pollutantId;
    Aggregation::Type _aggregationType;

    std::vector<std::string> _headers;
    QHash<QString, QRect> _mapping;
};
}