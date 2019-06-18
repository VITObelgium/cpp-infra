#include "uiinfra/maplegendmodel.h"

#include "infra/cast.h"

namespace inf::ui {

using namespace inf;

MapLegendModel::MapLegendModel(QObject* parent)
: QAbstractListModel(parent)
{
}

QHash<int, QByteArray> MapLegendModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole]  = "Name";
    roles[ColorRole] = "Color";
    return roles;
}

int MapLegendModel::rowCount(const QModelIndex& /*parent*/) const
{
    return truncate<int>(_items.size());
}

int MapLegendModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QVariant MapLegendModel::data(const QModelIndex& index, int role) const
{
    if (index.column() != 0) {
        return QVariant();
    }

    try {
        auto& item = _items.at(size_t(index.row()));

        if (role == NameRole) {
            return item.name();
        } else if (role == ColorRole) {
            return item.color();
        }

        if (role == Qt::DisplayRole) {
            return item.name();
        } else if (role == Qt::DecorationRole) {
            return item.color();
        }
    } catch (const std::out_of_range&) {
    }

    return QVariant();
}

void MapLegendModel::addLegendItem(const LegendItem& loc)
{
    beginResetModel();
    _items.push_back(loc);
    endResetModel();
}

void MapLegendModel::setLegendItems(std::vector<LegendItem> loc)
{
    beginResetModel();
    _items = std::move(loc);
    endResetModel();
}

void MapLegendModel::clear()
{
    beginResetModel();
    _items.clear();
    endResetModel();
}

}
