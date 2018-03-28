#include "uiinfra/sectionvisibilityselection.h"

#include <qheaderview.h>
#include <qmenu.h>
#include <qtableview.h>
#include <qtreeview.h>

namespace uiinfra {

void setSectionVisibilitySelector(QTableView* tableView)
{
    auto* header = tableView->horizontalHeader();
    header->setContextMenuPolicy(Qt::CustomContextMenu);

    QObject::connect(header, &QHeaderView::customContextMenuRequested, [header, tableView](const QPoint& pos) {
        QMenu contextMenu(QObject::tr("Zichtbare kolommen"), header);

        for (int i = 0; i < header->count(); ++i) {
            auto name = tableView->model()->headerData(i, Qt::Orientation::Horizontal).toString();

            auto action = new QAction(name, header);
            action->setCheckable(true);
            action->setChecked(!header->isSectionHidden(i));

            QObject::connect(action, &QAction::triggered, [header, index = i]() {
                header->setSectionHidden(index, !header->isSectionHidden(index));
            });
            contextMenu.addAction(action);
        }

        contextMenu.exec(tableView->mapToGlobal(pos));
    });
}

void setSectionVisibilitySelector(QTreeView* treeView)
{
    auto* header = treeView->header();
    header->setContextMenuPolicy(Qt::CustomContextMenu);

    QObject::connect(header, &QHeaderView::customContextMenuRequested, [header, treeView](const QPoint& pos) {
        QMenu contextMenu(QObject::tr("Zichtbare kolommen"), header);

        for (int i = 1; i < header->count(); ++i) {
            auto name = treeView->model()->headerData(i, Qt::Orientation::Horizontal).toString();

            auto action = new QAction(name, header);
            action->setCheckable(true);
            action->setChecked(!header->isSectionHidden(i));

            QObject::connect(action, &QAction::triggered, [header, index = i]() {
                header->setSectionHidden(index, !header->isSectionHidden(index));
            });
            contextMenu.addAction(action);
        }

        contextMenu.exec(treeView->mapToGlobal(pos));
    });
}
}
