#pragma once

#include <qglobal.h>

QT_FORWARD_DECLARE_CLASS(QItemSelectionModel)

namespace uiinfra {

void itemSelectionToClipboard(const QItemSelectionModel* selectionModel);
}
