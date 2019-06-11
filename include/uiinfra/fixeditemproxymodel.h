#pragma once

#include <qabstractproxymodel.h>

namespace inf::ui {

/*! Proxy model that inserts fixed items before the source model
 * Only one level of the source model will be provided
 * So the result will be a list model, not a tree model
 */
class FixedItemProxyModel : public QAbstractProxyModel
{
    Q_OBJECT

public:
    enum Role
    {
        FixedItemRole = Qt::UserRole + 10000,
    };

    FixedItemProxyModel(QObject* parent = nullptr);

    void setFixedItems(const QStringList& items);
    int fixedItemCount() const;

    // The index of the source model that is the root of the items
    void setRootModelIndex(const QModelIndex& root);
    QModelIndex rootModelIndex() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& parent) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

private:
    QStringList _items;
    QModelIndex _rootIndex;
};
}
