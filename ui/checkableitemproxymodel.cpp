#include "uiinfra/checkableitemproxymodel.h"

#include <cassert>

namespace inf::ui {

CheckableItemProxyModel::CheckableItemProxyModel(QObject* parent)
: QIdentityProxyModel(parent)
{
}

Qt::ItemFlags CheckableItemProxyModel::flags(const QModelIndex& proxyIndex) const
{
    auto flags = QIdentityProxyModel::flags(proxyIndex);
    flags.setFlag(Qt::ItemIsUserCheckable, true);
    return flags;
}

QVariant CheckableItemProxyModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::CheckStateRole) {
        auto iter = _checkStates.find(index);
        return iter == _checkStates.end() ? Qt::Unchecked : *iter;
    }

    return QIdentityProxyModel::data(index, role);
}

bool CheckableItemProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole) {
        _checkStates[index] = value.value<Qt::CheckState>();
        emit dataChanged(index, index, {Qt::CheckStateRole});
        return true;
    }

    return QIdentityProxyModel::setData(index, value, role);
}

}
