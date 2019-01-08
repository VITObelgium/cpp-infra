#include "uiinfra/uniquevaluesproxymodel.h"

#include <qdebug.h>

namespace uiinfra {

UniqueValuesProxyModel::UniqueValuesProxyModel(QObject* parent)
: QSortFilterProxyModel(parent)
{
}

QVariant UniqueValuesProxyModel::data(const QModelIndex& index, int role) const
{
    return mapToSource(index).data(role);
}

bool UniqueValuesProxyModel::emptyItemsAllowed() const
{
    return _emptyValues;
}

bool UniqueValuesProxyModel::filterAcceptsRow(int row, const QModelIndex& /*parent*/) const
{
    return isDuplicate(row);
}

bool UniqueValuesProxyModel::insertRows(int row, int count, const QModelIndex& parent)
{
    return QSortFilterProxyModel::insertRows(row, count, parent);
}

int UniqueValuesProxyModel::modelColumn() const
{
    return _modelColumn;
}

void UniqueValuesProxyModel::setModelColumn(int colum)
{
    beginResetModel();
    _modelColumn = colum;
    buildMap();
    endResetModel();
}

void UniqueValuesProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    connect(sourceModel, SIGNAL(layoutChanged()), this, SLOT(buildMap()));
    connect(sourceModel, SIGNAL(modelReset()), this, SLOT(buildMap()));
    connect(sourceModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(sourceModelDataChanged(QModelIndex, QModelIndex)));
    QSortFilterProxyModel::setSourceModel(sourceModel);
    buildMap();
    invalidate();
}

bool UniqueValuesProxyModel::isDuplicate(int row) const
{
    QVariant v = sourceModel()->index(row, _modelColumn).data(filterRole());
    if (!_emptyValues) {
        if (v.toString().isEmpty()) {
            return false;
        }
    }
    QMap<QString, QList<int>>::ConstIterator mIt = _valueMap.constFind(v.toString());
    if (mIt == _valueMap.constEnd()) {
        return true;
    }
    if (mIt.value().first() == row) {
        return true;
    }
    return false;
}

void UniqueValuesProxyModel::buildMap()
{
    beginResetModel();
    _valueMap.clear();
    if (sourceModel() == 0) {
        return;
    }
    QMap<QString, QList<int>>::Iterator it;
    int c = sourceModel()->rowCount();
    for (int iRow = 0; iRow < c; iRow++) {
        QVariant v = sourceModel()->index(iRow, _modelColumn).data(filterRole());
        it         = _valueMap.find(v.toString());
        if (it == _valueMap.end()) {
            _valueMap[v.toString()] = {iRow};
        } else {
            it.value().append(iRow);
        }
    }

    endResetModel();
    invalidate();
}

void UniqueValuesProxyModel::setEmptyItemsAllowed(bool on)
{
    if (_emptyValues != on) {
        buildMap();
    }
}

void UniqueValuesProxyModel::sourceModelDataChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/)
{
    buildMap();
}
}
