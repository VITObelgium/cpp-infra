#pragma once

#include <QStyledItemDelegate>

namespace uiinfra {

class FloatingPointItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FloatingPointItemDelegate(int decimals, QObject* parent = 0);
    virtual QString displayText(const QVariant& value, const QLocale& locale) const override;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;

private:
    int _decimals;
};

class EmptyZeroItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit EmptyZeroItemDelegate(QObject* parent = 0);
    virtual QString displayText(const QVariant& value, const QLocale& locale) const override;
};
}
