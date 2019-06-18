#include "uiinfra/checkablecombobox.h"
#include "infra/log.h"

#include <cassert>
#include <qapplication.h>
#include <qevent.h>
#include <qheaderview.h>
#include <qmetaobject.h>
#include <qstylepainter.h>
#include <qtableview.h>

namespace inf::ui {

MyQStyledItemDelegate::MyQStyledItemDelegate(QObject* parent)
: QStyledItemDelegate(parent)
{
}

void MyQStyledItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
}

CheckableComboBox::CheckableComboBox(QWidget* widget)
: QComboBox(widget)
, _tableWidget(new QTableView())
{
    _tableWidget->setModel(model());
    _tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    _tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    _tableWidget->horizontalHeader()->hide();
    _tableWidget->verticalHeader()->hide();
    _tableWidget->verticalHeader()->setMinimumSectionSize(16);
    _tableWidget->verticalHeader()->setDefaultSectionSize(16);
    _tableWidget->horizontalHeader()->setMinimumSectionSize(-1);
    _tableWidget->horizontalHeader()->setDefaultSectionSize(40);

    _tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    _tableWidget->setShowGrid(false);
    _tableWidget->setItemDelegateForColumn(0, new MyQStyledItemDelegate());
    setView(_tableWidget);

    _tableWidget->viewport()->installEventFilter(this);
}

void CheckableComboBox::setModel(QAbstractItemModel* model)
{
    _tableWidget->setModel(model);
    QComboBox::setModel(model);
}

QTableView* CheckableComboBox::tableWidget()
{
    return _tableWidget;
}

bool CheckableComboBox::eventFilter(QObject* object, QEvent* ev)
{
    if (ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseButtonRelease) {
        auto mousePos = static_cast<QMouseEvent*>(ev)->pos();
        auto index    = view()->indexAt(mousePos);
        auto vrect    = view()->visualRect(index);

        if ((model()->flags(index) & Qt::ItemIsUserCheckable) && vrect.contains(mousePos)) {
            auto style = _tableWidget->style();
            QStyleOptionViewItem option;
            auto* itemDelegate = qobject_cast<MyQStyledItemDelegate*>(_tableWidget->itemDelegateForColumn(0));
            if (itemDelegate) {
                itemDelegate->initStyleOption(&option, index);

                auto itemRect              = _tableWidget->visualRect(index);
                auto checkBoxIndicatorRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &option);
                checkBoxIndicatorRect.moveTop(itemRect.y() + checkBoxIndicatorRect.x());

                if (checkBoxIndicatorRect.contains(mousePos)) {
                    if (ev->type() == QEvent::MouseButtonRelease) {
                        // consume on release within the checkbox
                        auto newCheckState = index.data(Qt::CheckStateRole).value<Qt::CheckState>() == Qt::Checked ? Qt::Unchecked : Qt::Checked;
                        model()->setData(index, newCheckState, Qt::CheckStateRole);
                    }

                    // consume on click and release within the checkbox
                    return true;
                }
            }
        }
    }

    return QComboBox::eventFilter(object, ev);
}
}
