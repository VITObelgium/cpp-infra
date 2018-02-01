#include "uiinfra/checkcombobox.h"
#include <qabstractitemview.h>
#include <qevent.h>
#include <qlineedit.h>

namespace uiinfra {

CheckComboBox::CheckComboBox(QWidget* parent)
: QComboBox(parent)
{
    auto* checkModel = new CheckComboModel(this);

    setModel(checkModel);
    connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &CheckComboBox::toggleCheckState);
    connect(checkModel, &CheckComboModel::checkStateChanged, this, &CheckComboBox::updateCheckedItems);
    connect(checkModel, &CheckComboModel::rowsInserted, this, &CheckComboBox::updateCheckedItems);
    connect(checkModel, &CheckComboModel::rowsRemoved, this, &CheckComboBox::updateCheckedItems);

    // read-only contents
    QLineEdit* lineEdit = new QLineEdit(this);
    lineEdit->setReadOnly(true);
    setLineEdit(lineEdit);
    lineEdit->disconnect(this);
    setInsertPolicy(QComboBox::NoInsert);

    view()->installEventFilter(this);
    view()->window()->installEventFilter(this);
    view()->viewport()->installEventFilter(this);
    this->installEventFilter(this);
}

void CheckComboBox::hidePopup()
{
    if (_containerMousePress) {
        QComboBox::hidePopup();
    }
}

Qt::CheckState CheckComboBox::itemCheckState(int index) const
{
    return static_cast<Qt::CheckState>(itemData(index, Qt::CheckStateRole).toInt());
}

void CheckComboBox::setItemCheckState(int index, Qt::CheckState state)
{
    setItemData(index, state, Qt::CheckStateRole);
}

QStringList CheckComboBox::checkedItems() const
{
    QStringList items;
    if (model()) {
        QModelIndex index       = model()->index(0, modelColumn(), rootModelIndex());
        QModelIndexList indexes = model()->match(index, Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly);
        foreach (const QModelIndex& modelIndex, indexes) {
            items += modelIndex.data().toString();
        }
    }
    return items;
}

void CheckComboBox::setCheckedItems(const QStringList& items)
{
    foreach (const QString& text, items) {
        const int index = findText(text);
        setItemCheckState(index, index != -1 ? Qt::Checked : Qt::Unchecked);
    }
}

QString CheckComboBox::defaultText() const
{
    return _defaultText;
}

void CheckComboBox::setDefaultText(const QString& text)
{
    if (_defaultText != text) {
        setEditText(_defaultText);
        _defaultText = text;
    }
}

void CheckComboBox::updateCheckedItems()
{
    setEditText(_defaultText);
    emit checkedItemsChanged(checkedItems());
}

void CheckComboBox::toggleCheckState(int index)
{
    QVariant value = itemData(index, Qt::CheckStateRole);
    if (value.isValid()) {
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        setItemData(index, (state == Qt::Unchecked ? Qt::Checked : Qt::Unchecked), Qt::CheckStateRole);
    }
}

CheckComboModel::CheckComboModel(QObject* parent)
: QStandardItemModel(0 /*rows*/, 1 /*cols*/, parent)
{
}

Qt::ItemFlags CheckComboModel::flags(const QModelIndex& index) const
{
    return QStandardItemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant CheckComboModel::data(const QModelIndex& index, int role) const
{
    QVariant value = QStandardItemModel::data(index, role);
    if (index.isValid() && role == Qt::CheckStateRole && !value.isValid()) {
        value = Qt::Unchecked;
    }
    return value;
}

bool CheckComboModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool ok = QStandardItemModel::setData(index, value, role);
    if (ok && role == Qt::CheckStateRole) {
        emit dataChanged(index, index);
        emit checkStateChanged();
    }
    return ok;
}

// the filter is needed to avoid closing the combobox popup every time a checkbox is toggled
bool CheckComboBox::eventFilter(QObject* receiver, QEvent* event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (receiver == this && (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)) {
            showPopup();
            return true;
        } else if (keyEvent->key() == Qt::Key_Enter ||
                   keyEvent->key() == Qt::Key_Return ||
                   keyEvent->key() == Qt::Key_Escape) {
            // it is important to call QComboBox implementation
            QComboBox::hidePopup();
            if (keyEvent->key() != Qt::Key_Escape) {
                return true;
            }
        }
    }
    case QEvent::MouseButtonPress:
        _containerMousePress = (receiver == view()->window());
        break;
    case QEvent::MouseButtonRelease:
        _containerMousePress = false;
        break;
    default:
        break;
    }
    return false;
}
}
