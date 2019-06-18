#pragma once

#include <qcombobox.h>
#include <qstyleditemdelegate.h>

QT_FORWARD_DECLARE_CLASS(QTableView)

namespace inf::ui {

class MyQStyledItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MyQStyledItemDelegate(QObject* parent = nullptr);

    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
};

class CheckableComboBox : public QComboBox
{
    Q_OBJECT

public:
    CheckableComboBox(QWidget* widget = nullptr);

    void setModel(QAbstractItemModel* model);
    QTableView* tableWidget();

protected:
    bool eventFilter(QObject* object, QEvent* ev);

private:
    QTableView* _tableWidget;
};
}
