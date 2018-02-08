#pragma once

#include <qabstractproxymodel.h>

namespace uiinfra {

class FixedItemProxyModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    FixedItemProxyModel(QObject* parent = nullptr);

    void setFixedItems(const QStringList& items);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& parent) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

private:
    QStringList _items;
};
}
