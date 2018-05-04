#pragma once

#include "DateTime.h"
#include "Aggregation.h"

#include <QAbstractTableModel>
#include <QRect>

namespace opaq
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
                       chrono::date_time baseTime,
                       const std::string& stationName,
                       chrono::days forecastHorizon,
                       const std::string& pollutantId,
                       Aggregation::Type agg);

    void addMapping(QString color, QRect area);

private:
    ForecastBuffer* _buffer;
    int _rowCount;
    int _colCount;
    chrono::date_time _baseTime;
    chrono::days _forecastHorizon;
    std::string _stationName;
    std::string _pollutantId;
    Aggregation::Type _aggregationType;

    std::vector<std::string> _headers;
    QHash<QString, QRect> _mapping;
};
}