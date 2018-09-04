#pragma once

#include "infra/cast.h"
#include "uiinfra/clearableabstractitemmodel.h"

#include <qvariant.h>

namespace uiinfra {

class TreeNode
{
public:
    TreeNode()  = default;
    ~TreeNode() = default;

    TreeNode(TreeNode& parent)
    : _parent(&parent)
    {
    }

    int childCount() const
    {
        return inf::truncate<int>(_children.size());
    }

    TreeNode* parent()
    {
        return _parent;
    }

    const TreeNode* parent() const
    {
        return _parent;
    }

    TreeNode* child(int index)
    {
        return index < _children.size() ? _children[index].get() : nullptr;
    }

    int indexOf(const TreeNode* node) const
    {
        auto iter = std::find_if(_children.begin(), _children.end(), [node](auto& ptr) {
            return ptr.get() == node;
        });

        if (iter == _children.end()) {
            return -1;
        }

        return inf::truncate<int>(std::distance(_children.begin(), iter));
    }

    int row() const
    {
        if (_parent) {
            return _parent->indexOf(this);
        }

        return 0;
    }

    template <typename T>
    T* addChild(std::unique_ptr<T> child)
    {
        return static_cast<T*>(_children.emplace_back(std::move(child)).get());
    }

    virtual QVariant data(int column, int role) const                 = 0;
    virtual bool setData(int column, const QVariant& value, int role) = 0;

    virtual void updateFlags(QModelIndex /*index*/, Qt::ItemFlags& /*flags*/) const
    {
    }

private:
    TreeNode* _parent = nullptr;
    std::vector<std::unique_ptr<TreeNode>> _children;
};

class RootNode : public TreeNode
{
public:
    RootNode()
    {
    }

    virtual QVariant data(int /*column*/, int /*role*/) const override
    {
        return QVariant();
    }

    virtual bool setData(int /*column*/, const QVariant& /*value*/, int /*role*/) override
    {
        return false;
    }
};

class TreeModel : public ClearableAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(QObject* parent = nullptr);
    ~TreeModel() override;

    void setHeaders(std::vector<QString> headers);
    void setRootNode(std::unique_ptr<TreeNode> root);
    virtual void clear() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void populateTree();

    template <typename NodeType>
    void populateSubstanceGroup(TreeNode& parent);

    std::unique_ptr<TreeNode> _root;
    std::vector<QString> _headers;
};
}
