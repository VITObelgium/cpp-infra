#pragma once

#include "qidentityproxymodel.h"

QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QTreeView)

namespace uiinfra {

void setSectionVisibilitySelector(QTableView* tableView);
void setSectionVisibilitySelector(QTreeView* treeView);
}
