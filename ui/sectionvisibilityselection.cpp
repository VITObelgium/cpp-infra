#include "uiinfra/sectionvisibilityselection.h"
#include "infra/algo.h"

#include <qheaderview.h>
#include <qmenu.h>
#include <qtableview.h>
#include <qtreeview.h>

namespace uiinfra {

void setSectionVisibilitySelector(QHeaderView* headerView, const std::vector<int>& fixedSections)
{
    headerView->setContextMenuPolicy(Qt::CustomContextMenu);

    QObject::connect(headerView, &QHeaderView::customContextMenuRequested, [headerView, fixedSections](const QPoint& pos) {
        QMenu contextMenu(QObject::tr("Zichtbare kolommen"), headerView);

        for (int i = 0; i < headerView->count(); ++i) {
            if (inf::containerContains(fixedSections, i)) {
                continue;
            }

            auto name = headerView->model()->headerData(i, Qt::Orientation::Horizontal).toString();

            auto action = new QAction(name, headerView);
            action->setCheckable(true);
            action->setChecked(!headerView->isSectionHidden(i));

            QObject::connect(action, &QAction::triggered, [headerView, index = i]() {
                headerView->setSectionHidden(index, !headerView->isSectionHidden(index));
            });
            contextMenu.addAction(action);
        }

        contextMenu.exec(headerView->mapToGlobal(pos));
    });
}
}
