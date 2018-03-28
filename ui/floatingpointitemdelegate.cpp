#include "uiinfra/floatingpointitemdelegate.h"

#include <qspinbox.h>

namespace uiinfra {

FloatingPointItemDelegate::FloatingPointItemDelegate(int decimals, QObject* parent)
: QStyledItemDelegate(parent)
, _decimals(decimals)
{
}

QString FloatingPointItemDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    QString formattedNum = locale.toString(value.toDouble(), 'g', 15);
    return formattedNum;
}

void FloatingPointItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* comboBox = static_cast<QDoubleSpinBox*>(editor);
    auto value     = index.model()->data(index, Qt::EditRole).toDouble();
    comboBox->setDecimals(_decimals);
    comboBox->setValue(value);
}
}
