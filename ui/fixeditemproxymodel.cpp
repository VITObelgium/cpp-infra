#include "uiinfra/fixeditemproxymodel.h"

#include <cassert>

namespace inf::ui {

FixedItemProxyModel::FixedItemProxyModel(QObject* parent)
: QAbstractProxyModel(parent)
{
}

void FixedItemProxyModel::setFixedItems(const QStringList& items)
{
    _items = items;
}

int FixedItemProxyModel::fixedItemCount() const
{
    return _items.size();
}

void FixedItemProxyModel::setRootModelIndex(const QModelIndex& root)
{
    beginResetModel();
    _rootIndex = root;
    endResetModel();
}

QModelIndex FixedItemProxyModel::rootModelIndex() const
{
    return _rootIndex;
}

int FixedItemProxyModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    assert(!parent.isValid()); // don't expect this to work with hierarchies
    if (!sourceModel()) {
        return _items.size();
    }

    return sourceModel()->rowCount(_rootIndex) + _items.size();
}

int FixedItemProxyModel::columnCount(const QModelIndex& parent) const
{
    if (!sourceModel()) {
        return 1;
    }

    return sourceModel()->columnCount(parent);
}

QModelIndex FixedItemProxyModel::parent(const QModelIndex& /*index*/) const
{
    return QModelIndex();
}

Qt::ItemFlags FixedItemProxyModel::flags(const QModelIndex& proxyIndex) const
{
    if (proxyIndex.row() < _items.count()) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    if (!sourceModel()) {
        return Qt::NoItemFlags;
    }

    return sourceModel()->flags(mapToSource(proxyIndex));
}

QVariant FixedItemProxyModel::data(const QModelIndex& proxyIndex, int role) const
{
    assert(!proxyIndex.parent().isValid());
    bool isFixedItem = proxyIndex.row() < _items.count();
    if (role == FixedItemRole) {
        return isFixedItem;
    }

    if (isFixedItem) {
        if (role != Qt::DisplayRole) {
            return QVariant();
        }

        return _items.at(proxyIndex.row());
    }

    if (!sourceModel()) {
        return QVariant();
    }

    return sourceModel()->data(mapToSource(proxyIndex), role);
}

QModelIndex FixedItemProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    assert(!parent.isValid());
    return createIndex(row, column);
}

QModelIndex FixedItemProxyModel::mapFromSource(const QModelIndex& sourceIndex) const
{
    if (!sourceModel()) {
        return QModelIndex();
    }

    return createIndex(sourceIndex.row() + _items.size(), 0);
}

QModelIndex FixedItemProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!sourceModel() || proxyIndex.row() < _items.size()) {
        // Fixed items do not appear in the source model
        return QModelIndex();
    }

    auto column = _rootIndex.isValid() ? _rootIndex.column() : 0;
    column += proxyIndex.column();
    return sourceModel()->index(proxyIndex.row() - _items.size(), column, _rootIndex);
}
}
