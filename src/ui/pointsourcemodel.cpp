#include "pointsourcemodel.h"
#include "infra/cast.h"
#include "infra/gdal.h"
#include "uiinfra/colorconversion.h"

#include <QColor>

namespace opaq {

using namespace inf;

PointSourceModel::PointSourceModel(QObject* parent)
: QAbstractTableModel(parent)
{
}

QHash<int, QByteArray> PointSourceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole]      = "Name";
    roles[ValueRole]     = "Value";
    roles[LongitudeRole] = "Longitude";
    roles[LatitudeRole]  = "Latitude";
    roles[ColorRole]     = "Color";
    return roles;
}

int PointSourceModel::rowCount(const QModelIndex& /*parent*/) const
{
    return truncate<int>(_data.size());
}

int PointSourceModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant PointSourceModel::data(const QModelIndex& index, int role) const
{
    if (index.row() >= truncate<int>(_data.size()) || index.column() != 0) {
        return QVariant();
    }

    if (role == NameRole) {
        return _data[index.row()].name;
    } else if (role == ValueRole) {
        return _data[index.row()].value;
    } else if (role == LatitudeRole) {
        return _data[index.row()].latitude;
    } else if (role == LongitudeRole) {
        return _data[index.row()].longitude;
    } else if (role == ColorRole) {
        return colorValue(_data[index.row()].value);
    }

    return QVariant();
}

void PointSourceModel::setLegend(const Legend& legend)
{
    _legend = legend;

    if (_legend.entries.empty()) {
        return;
    }

    auto iter = std::max_element(_legend.entries.begin(), _legend.entries.end(), [](auto& lhs, auto& rhs) {
        return lhs.upperBound < rhs.upperBound;
    });

    _maxValue = iter->upperBound;
    _minColor = uiinfra::toQColor(_legend.entries.front().color);
    _maxColor = uiinfra::toQColor(iter->color);

    int rows = rowCount();
    if (rows > 0) {
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {ColorRole});
    }
}

void PointSourceModel::setModelData(std::vector<PointSourceModelData> data)
{
    if (data.size() != _data.size()) {
        // TODO: investigate: this causes memory leak inside the qml engine
        beginResetModel();
        _data = std::move(data);
        endResetModel();
    } else {
        _data = std::move(data);
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    }
}

void PointSourceModel::clear()
{
    beginResetModel();
    _data.clear();
    endResetModel();
}

QColor PointSourceModel::colorValue(float value) const
{
    if (std::isnan(value)) {
        return QColor(0, 0, 0);
    }

    for (auto& entry : _legend.entries) {
        if (value >= entry.lowerBound && value < entry.upperBound) {
            return uiinfra::toQColor(entry.color);
        }
    }

    if (value >= _maxValue) {
        return _maxColor;
    }

    return _minColor;
}
}
