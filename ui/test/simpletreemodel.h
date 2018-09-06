#pragma once

#include <memory>
#include <qabstractitemmodel.h>
#include <qvariant.h>

namespace uiinfra::test {

class TreeItem
{
public:
    TreeItem(const QList<QVariant>& data, TreeItem* parent = nullptr)
    {
        _parentItem = parent;
        _itemData   = data;
    }

    ~TreeItem()
    {
        qDeleteAll(_childItems);
    }

    void appendChild(TreeItem* item)
    {
        _childItems.append(item);
    }

    TreeItem* child(int row)
    {
        return _childItems.value(row);
    }

    int childCount() const
    {
        return _childItems.count();
    }

    int columnCount() const
    {
        return _itemData.count();
    }

    QVariant data(int column) const
    {
        return _itemData.value(column);
    }

    TreeItem* parentItem()
    {
        return _parentItem;
    }

    int row() const
    {
        if (_parentItem)
            return _parentItem->_childItems.indexOf(const_cast<TreeItem*>(this));

        return 0;
    }

private:
    QList<TreeItem*> _childItems;
    QList<QVariant> _itemData;
    TreeItem* _parentItem;
};

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TreeModel(const QString& data, QObject* parent = nullptr)
    : QAbstractItemModel(parent)
    {
        QList<QVariant> rootData;
        rootData << "Title"
                 << "Summary";
        _rootItem = std::make_unique<TreeItem>(rootData);
        setupModelData(data.split(QString("\n")), _rootItem.get());
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid())
            return QVariant();

        if (role != Qt::DisplayRole)
            return QVariant();

        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());

        return item->data(index.column());
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if (!index.isValid())
            return 0;

        return QAbstractItemModel::flags(index);
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return _rootItem->data(section);

        return QVariant();
    }

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override
    {
        if (!hasIndex(row, column, parent))
            return QModelIndex();

        TreeItem* parentItem;

        if (!parent.isValid())
            parentItem = _rootItem.get();
        else
            parentItem = static_cast<TreeItem*>(parent.internalPointer());

        TreeItem* childItem = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    }

    QModelIndex parent(const QModelIndex& index) const override
    {
        if (!index.isValid())
            return QModelIndex();

        TreeItem* childItem  = static_cast<TreeItem*>(index.internalPointer());
        TreeItem* parentItem = childItem->parentItem();

        if (parentItem == _rootItem.get())
            return QModelIndex();

        return createIndex(parentItem->row(), 0, parentItem);
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        TreeItem* parentItem;
        if (parent.column() > 0)
            return 0;

        if (!parent.isValid())
            parentItem = _rootItem.get();
        else
            parentItem = static_cast<TreeItem*>(parent.internalPointer());

        return parentItem->childCount();
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid())
            return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
        else
            return _rootItem->columnCount();
    }

private:
    void setupModelData(const QStringList& lines, TreeItem* parent)
    {
        QList<TreeItem*> parents;
        QList<int> indentations;
        parents << parent;
        indentations << 0;

        int number = 0;

        while (number < lines.count()) {
            int position = 0;
            while (position < lines[number].length()) {
                if (lines[number].at(position) != ' ')
                    break;
                position++;
            }

            QString lineData = lines[number].mid(position).trimmed();

            if (!lineData.isEmpty()) {
                // Read the column data from the rest of the line.
                QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
                QList<QVariant> columnData;
                for (int column = 0; column < columnStrings.count(); ++column)
                    columnData << columnStrings[column];

                if (position > indentations.last()) {
                    // The last child of the current parent is now the new parent
                    // unless the current parent has no children.

                    if (parents.last()->childCount() > 0) {
                        parents << parents.last()->child(parents.last()->childCount() - 1);
                        indentations << position;
                    }
                } else {
                    while (position < indentations.last() && parents.count() > 0) {
                        parents.pop_back();
                        indentations.pop_back();
                    }
                }

                // Append a new item to the current parent's list of children.
                parents.last()->appendChild(new TreeItem(columnData, parents.last()));
            }

            ++number;
        }
    }

    std::unique_ptr<TreeItem> _rootItem;
};
}
