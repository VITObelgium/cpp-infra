#pragma once

#include "Engine.h"

#include <QAbstractTableModel>

namespace OPAQ
{

class ValidationResultsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ValidationResultsModel(QObject* parent);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void addResult(const std::string& modelName, std::vector<PredictionResult> result);
    void clear();

private:
    std::vector<std::vector<PredictionResult>> _results;
    std::vector<std::string> _headers;

    int _rowCount;
    int _colCount;
};
}