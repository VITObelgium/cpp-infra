#pragma once

#include <qstringlist.h>
#include <qstyleditemdelegate.h>

class QModelIndex;
class QWidget;
class QVariant;

namespace inf::ui {

class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ComboBoxDelegate(QObject* parent = nullptr);

    void setItems(const QStringList& items);
    void setModel(QAbstractItemModel* model);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private:
    QStringList _items;
    QAbstractItemModel* _model;
};
}
