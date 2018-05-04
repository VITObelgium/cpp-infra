#pragma once

#include "Engine.h"

#include <QAbstractTableModel>

namespace opaq
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

    double getRootMeanSquareError(const std::string& modelName) const;
    double getBias(const std::string& modelName) const;
    double getRSquare(const std::string& modelName) const;

    void addResult(const std::string& modelName, std::vector<PredictionResult> result);
    void clear();

private:
    std::vector<std::vector<PredictionResult>> _results;
    std::vector<std::string> _headers;
    std::map<std::string, double> _rmse;
    std::map<std::string, double> _bias;
    std::map<std::string, double> _rSquare;

    int _rowCount;
    int _colCount;
};
}