#include "predictionresultsmodel.h"

#include "data/ForecastBuffer.h"

namespace OPAQ
{

PredictionResultsModel::PredictionResultsModel(QObject *parent)
: QAbstractTableModel(parent)
, _buffer(nullptr)
{
}

int PredictionResultsModel::rowCount(const QModelIndex & /*parent*/) const
{
    return 2;
}

int PredictionResultsModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 6;
}

QVariant PredictionResultsModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        return QString("Row%1, Column%2")
            .arg(index.row() + 1)
            .arg(index.column() + 1);
    }
    
    return QVariant();
}

void PredictionResultsModel::updateResults(ForecastBuffer& buffer)
{
    _buffer = &buffer;
    //TODO: update model information based on the buffer
}

}