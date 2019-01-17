#pragma once

#include <qglobal.h>

QT_FORWARD_DECLARE_CLASS(QItemSelectionModel)

namespace inf::ui {

void itemSelectionToClipboard(const QItemSelectionModel* selectionModel, char doubleFormat = 'f', int precision = 10);
}
