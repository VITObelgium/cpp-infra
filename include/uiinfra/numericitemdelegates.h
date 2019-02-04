#pragma once

#include <QStyledItemDelegate>

namespace inf::ui {

class IntegerItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit IntegerItemDelegate(QObject* parent = nullptr);
    IntegerItemDelegate(int minValue, int maxValue, QObject* parent = nullptr);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual QString displayText(const QVariant& value, const QLocale& locale) const override;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;

private:
    int _minValue = 0;
    int _maxValue = 100;
};

class FloatingPointItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FloatingPointItemDelegate(char format, int decimals, QObject* parent = nullptr);
    virtual QString displayText(const QVariant& value, const QLocale& locale) const override;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;

private:
    char _format;
    int _decimals;
};

class EmptyZeroItemDelegate : public IntegerItemDelegate
{
    Q_OBJECT
public:
    explicit EmptyZeroItemDelegate(QObject* parent = nullptr);
    EmptyZeroItemDelegate(int minValue, int maxValue, QObject* parent = nullptr);
    virtual QString displayText(const QVariant& value, const QLocale& locale) const override;
};
}
