#include "uiinfra/comboboxdelegate.h"
#include "infra/log.h"
#include <QComboBox>
#include <QModelIndex>

namespace uiinfra {

ComboBoxDelegate::ComboBoxDelegate(QObject* parent)
: QStyledItemDelegate(parent)
, _model(nullptr)
{
}

void ComboBoxDelegate::setItems(const QStringList& items)
{
    _items = items;
    _model = nullptr;
}

void ComboBoxDelegate::setModel(QAbstractItemModel* model)
{
    _model = model;
    _items.clear();
}

QWidget* ComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /* option */, const QModelIndex& /* index */) const
{
    auto* editor = new QComboBox(parent);
    if (_model) {
        editor->setModel(_model);
    } else {
        for (const auto& item : _items) {
            editor->addItem(item);
        }
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
    infra::Log::info("Data: {} {}", comboBox->currentData(Qt::UserRole).toLongLong(), comboBox->currentText().toStdString());
    model->setData(index, comboBox->currentText(), Qt::DisplayRole);
    model->setData(index, comboBox->currentData(Qt::UserRole), Qt::UserRole);
}
}
