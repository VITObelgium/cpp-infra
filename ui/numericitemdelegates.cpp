#include "uiinfra/numericitemdelegates.h"

#include <qspinbox.h>

namespace inf::ui {

IntegerItemDelegate::IntegerItemDelegate(QObject* parent)
: QStyledItemDelegate(parent)
{
}

IntegerItemDelegate::IntegerItemDelegate(int minValue, int maxValue, QObject* parent)
: QStyledItemDelegate(parent)
, _minValue(minValue)
, _maxValue(maxValue)
{
}

QWidget* IntegerItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto sb = new QSpinBox(parent);
    sb->setMinimum(_minValue);
    sb->setMaximum(_maxValue);
    return sb;
}

QString IntegerItemDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    return locale.toString(value.toInt());
}

void IntegerItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* spinBox = static_cast<QSpinBox*>(editor);
    auto value    = index.model()->data(index, Qt::EditRole).toInt();
    spinBox->setValue(value);
}

FloatingPointItemDelegate::FloatingPointItemDelegate(char format, int decimals, QObject* parent)
: QStyledItemDelegate(parent)
, _format(format)
, _decimals(decimals)
{
}

QString FloatingPointItemDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const bool isDouble = value.type() == QVariant::Double;
#else
    const bool isDouble = value.metaType().id() == QMetaType::Double;
#endif

    if (isDouble) {
        return locale.toString(value.toDouble(), _format, _decimals);
    } else {
        return value.toString();
    }
}

void FloatingPointItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* spinBox = static_cast<QDoubleSpinBox*>(editor);
    auto value    = index.model()->data(index, Qt::EditRole).toDouble();
    spinBox->setDecimals(_decimals);
    spinBox->setValue(value);
}

EmptyZeroItemDelegate::EmptyZeroItemDelegate(QObject* parent)
: IntegerItemDelegate(parent)
{
}

EmptyZeroItemDelegate::EmptyZeroItemDelegate(int minValue, int maxValue, QObject* parent)
: IntegerItemDelegate(minValue, maxValue, parent)
{
}

QString EmptyZeroItemDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    if (value.toInt() == 0) {
        return QString();
    }

    return IntegerItemDelegate::displayText(value, locale);
}
}
