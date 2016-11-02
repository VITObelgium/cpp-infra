#include "validationresultsmodel.h"

#include "data/ForecastBuffer.h"
#include "tools/CalcTools.h"

namespace opaq
{

ValidationResultsModel::ValidationResultsModel(QObject* parent)
: QAbstractTableModel(parent)
, _rowCount(0)
, _colCount(0)
{
}

int ValidationResultsModel::rowCount(const QModelIndex& /*parent*/) const
{
    return _rowCount;
}

int ValidationResultsModel::columnCount(const QModelIndex& /*parent*/) const
{
    return _colCount;
}

QVariant ValidationResultsModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (index.row() == 0)
        {
            return index.column();
        }

        // We use the first row to store the indexes, so take that into account
        int adjustedRow = index.row() - 1;

        auto& results = _results.at(adjustedRow / 2);
        float value;
        if (adjustedRow % 2 == 0)
        {
            // measured values
            value = results.at(index.column()).measuredValue;
        }
        else
        {
            // predicted values
            value = results.at(index.column()).predictedValue;
        }

        if (std::fabs(value - -9999) < std::numeric_limits<double>::epsilon())
        {
            return tr("No data");
        }

        return QString::number(value, 'f', 6);
    }

    return QVariant();
}

QVariant ValidationResultsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal)
    {
        if (section == 0)
        {
            return QVariant();
        }

        return QString(_headers.at((section - 1) / 2).c_str());
    }
    else
    {
        return QString::number(section, 'd', 2);
    }
}

double ValidationResultsModel::getRootMeanSquareError(const std::string& modelName) const
{
    return _rmse.at(modelName);
}

double ValidationResultsModel::getBias(const std::string& modelName) const
{
    return _bias.at(modelName);
}

double ValidationResultsModel::getRSquare(const std::string& modelName) const
{
    return _rSquare.at(modelName);
}

void ValidationResultsModel::addResult(const std::string& modelName, std::vector<PredictionResult> result)
{
    if (_rowCount == 0)
    {
        ++_rowCount;
    }

    _rowCount += 2;
    _colCount = std::max(_colCount, static_cast<int>(result.size()));

    _rmse[modelName] = CalcTools::rmse(result);
    _bias[modelName] = CalcTools::bias(result);

    double slope, icept;
    _rSquare[modelName] = CalcTools::lsqreg(result, slope, icept);

    _headers.push_back(modelName);
    _results.emplace_back(std::move(result));
}

void ValidationResultsModel::clear()
{
    _rowCount = 0;
    _colCount = 0;
    _headers.clear();
    _results.clear();
    _rmse.clear();
    _bias.clear();
    _rSquare.clear();
}


}
