#include "uiinfra/fixeditemproxymodel.h"

namespace uiinfra {

FixedItemProxyModel::FixedItemProxyModel(QObject* parent)
: QAbstractProxyModel(parent)
{
}

void FixedItemProxyModel::setFixedItems(const QStringList& items)
{
    _items = items;
}

int FixedItemProxyModel::rowCount(const QModelIndex& parent) const
{
    return sourceModel()->rowCount(parent) + _items.size();
}

int FixedItemProxyModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid() && parent.row() < _items.count()) {
        return 1;
    }

    return sourceModel()->columnCount(parent);
}

QModelIndex FixedItemProxyModel::parent(const QModelIndex& index) const
{
    if (index.isValid() && index.row() < _items.count()) {
        sourceModel()->parent(sourceModel()->index(0, index.column()));
    }

    return sourceModel()->parent(mapToSource(index));
}

QVariant FixedItemProxyModel::data(const QModelIndex& proxyIndex, int role) const
{
    if (proxyIndex.row() < _items.count()) {
        if (role != Qt::DisplayRole) {
            return QVariant();
        }

        //Log::info("{} {} {}", proxyIndex.row(), proxyIndex.column(), _items.at(proxyIndex.row()).toStdString());
        return _items.at(proxyIndex.row());
    }

    return QAbstractProxyModel::data(proxyIndex, role);
}

QModelIndex FixedItemProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column, parent.internalId());
}

QModelIndex FixedItemProxyModel::mapFromSource(const QModelIndex& sourceIndex) const
{
    if (!sourceModel()) {
        return QModelIndex();
    }

    return sourceModel()->index(sourceIndex.row() + _items.size(), sourceIndex.column());
}

QModelIndex FixedItemProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!sourceModel() || proxyIndex.row() < _items.size()) {
        // Fixed items do not appear in the source model
        return QModelIndex();
    }

    return sourceModel()->index(proxyIndex.row() - _items.size(), proxyIndex.column());
}
}
