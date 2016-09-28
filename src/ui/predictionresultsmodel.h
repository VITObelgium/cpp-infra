#pragma once

#include <QAbstractTableModel>

namespace OPAQ
{

class ForecastBuffer;

class PredictionResultsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PredictionResultsModel(QObject *parent);
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void updateResults(ForecastBuffer& buffer);

private:
    ForecastBuffer* _buffer;
};

}