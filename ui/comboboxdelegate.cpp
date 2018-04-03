#include "uiinfra/comboboxdelegate.h"

#include <QComboBox>
#include <QModelIndex>

namespace uiinfra {

ComboBoxDelegate::ComboBoxDelegate(QStringList items, QObject* parent)
: QStyledItemDelegate(parent)
, _items(std::move(items))
{
}

QWidget* ComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /* option */, const QModelIndex& /* index */) const
{
    auto* editor = new QComboBox(parent);
    for (const auto& item : _items) {
        editor->addItem(item);
    }
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* comboBox = static_cast<QComboBox*>(editor);
    auto value     = index.model()->data(index, Qt::EditRole).toString();
    comboBox->setCurrentText(value);
}

void ComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}
}
