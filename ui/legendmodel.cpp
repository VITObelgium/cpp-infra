#include "uiinfra/legendmodel.h"

#include "infra/cast.h"
#include "uiinfra/colorconversion.h"

namespace inf::ui {

LegendModel::LegendModel(QObject* parent)
: QAbstractTableModel(parent)
{
    _legend.numberOfClasses = 0;
    _legend.type            = Legend::Type::Numeric;
}

QHash<int, QByteArray> LegendModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole]  = "Name";
    roles[ColorRole] = "Color";
    return roles;
}

int LegendModel::rowCount(const QModelIndex& /*parent*/) const
{
    if (_legend.entries.empty()) {
        return 0;
    }

    return truncate<int>(_legend.numberOfClasses);
}

int LegendModel::columnCount(const QModelIndex& /*parent*/) const
{
    return isNumeric() ? 4 : 3;
}

QVariant LegendModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        if (index.column() > 1) {
            return int(Qt::AlignRight | Qt::AlignVCenter);
        } else {
            return QVariant();
        }
    }

    if (index.row() >= truncate<int>(_legend.entries.size())) {
        return QVariant();
    }

    try {
        if ((role == Qt::BackgroundColorRole || role == ColorRole) && index.column() == 0) {
            return uiinfra::toQColor(_legend.entries[index.row()].color);
        }

        if (role != Qt::DisplayRole && role != Qt::EditRole && role != NameRole) {
            return QVariant();
        }

        auto& item = _legend.entries.at(index.row());

        switch (index.column()) {
        case 0:
            if (role == NameRole) {
                return QString::fromStdString(item.name);
            }
            break;
        case 1:
            return QString::fromStdString(item.name);
        case 2:
            if (isNumeric()) {
                return item.lowerBound;
            } else {
                return index.row();
            }
        case 3:
            if (isNumeric()) {
                return item.upperBound;
            }
        }
    } catch (const std::out_of_range&) {
    }

    return QVariant();
}

bool LegendModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    auto dataAccepted = false;
    auto& item        = _legend.entries.at(index.row());
    switch (index.column()) {
    case 0:
        if (role == Qt::BackgroundColorRole) {
            item.color = uiinfra::fromQColor(value.value<QColor>());
        }
        break;
    case 1:
        item.name    = value.toString().toStdString();
        dataAccepted = true;
        break;
    case 2:
        if (isNumeric()) {
            item.lowerBound = value.toDouble();
            dataAccepted    = true;
        }
        break;
    case 3:
        if (isNumeric()) {
            item.upperBound = value.toDouble();
            dataAccepted    = true;
        }
        break;
    }

    if (dataAccepted) {
        emit dataChanged(index, index);
    }

    return dataAccepted;
}

QVariant LegendModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return tr("Colour");
    case 1:
        return tr("Label");
    case 2:
        if (isNumeric()) {
            return tr("Lower bound");
        } else {
            return tr("Value");
        }
    case 3:
        return tr("Upper bound");
    }

    return QVariant();
}

Qt::ItemFlags LegendModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return nullptr;
    }

    auto flags = QAbstractItemModel::flags(index);
    if (index.column() == 1 || index.column() == 3 || (index.column() == 2 && isNumeric())) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

Legend LegendModel::legend() const
{
    Legend legendCopy = _legend;
    legendCopy.entries.resize(_legend.numberOfClasses);
    return legendCopy;
}

void LegendModel::setLegend(Legend legend)
{
    beginResetModel();
    _legend = std::move(legend);
    endResetModel();
}

void LegendModel::setLegendType(Legend::Type type)
{
    beginResetModel();
    _legend.type = type;
    endResetModel();
}

bool LegendModel::treatZeroAsNodata() const
{
    return _legend.zeroIsNodata;
}

void LegendModel::setTreatZeroAsNodata(bool enabled)
{
    _legend.zeroIsNodata = enabled;
}

int LegendModel::numberOfClasses() const
{
    return _legend.numberOfClasses;
}

void LegendModel::setNumberOfClasses(int count)
{
    beginResetModel();
    _legend.numberOfClasses = count;
    // Don't shrink the entries when reducing the count to avoid data loss
    // when the user decies to increase the count again
    if (count > truncate<int>(_legend.entries.size())) {
        _legend.entries.resize(count);
    }
    endResetModel();
}

QString LegendModel::legendTitle() const
{
    return QString::fromStdString(_legend.title);
}

void LegendModel::clear()
{
    beginResetModel();
    _legend.entries.clear();
    endResetModel();
}

bool LegendModel::isNumeric() const
{
    return _legend.type == Legend::Type::Numeric;
}

}
