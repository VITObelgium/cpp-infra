#include "uiinfra/treemodel.h"

#include "infra/enumutils.h"

#include <qapplication.h>

namespace inf::ui {

using namespace inf;

TreeModel::TreeModel(QObject* parent)
: ClearableAbstractItemModel(parent)
{
}

TreeModel::~TreeModel() = default;

void TreeModel::setHeaders(std::vector<QString> headers)
{
    _headers = std::move(headers);
}

void TreeModel::setRootNode(std::unique_ptr<TreeNode> root)
{
    this->beginResetModel();
    _root = std::move(root);
    this->endResetModel();
}

void TreeModel::clear()
{
    this->beginResetModel();
    _root.reset();
    this->endResetModel();
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0 || !_root) {
        return 0;
    }

    TreeNode* parentItem = parent.isValid() ? static_cast<TreeNode*>(parent.internalPointer()) : const_cast<TreeNode*>(_root.get());
    return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex& /*parent*/) const
{
    return truncate<int>(_headers.size());
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!_root) {
        return QModelIndex();
    }

    TreeNode* parentItem = parent.isValid() ? static_cast<TreeNode*>(parent.internalPointer()) : const_cast<TreeNode*>(_root.get());

    if (auto* child = parentItem->child(row); child != nullptr) {
        return createIndex(row, column, child);
    }

    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    auto* childItem  = static_cast<TreeNode*>(index.internalPointer());
    auto* parentItem = childItem->parent();

    if (parentItem == _root.get()) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    return static_cast<const TreeNode*>(index.internalPointer())->data(index.column(), role);
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    bool modified = static_cast<TreeNode*>(index.internalPointer())->setData(index.column(), value, role);
    if (modified) {
        emit dataChanged(index, index, {role});
    }

    return modified;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return nullptr;
    }

    auto flags = QAbstractItemModel::flags(index);
    static_cast<const TreeNode*>(index.internalPointer())->updateFlags(index, flags);
    if (!_editable) {
        flags.setFlag(Qt::ItemIsEditable, false);
    }
    return flags;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return _headers.at(section);
    }

    return QVariant();
}

void TreeModel::setEditable(bool enabled)
{
    _editable = enabled;
}

}
