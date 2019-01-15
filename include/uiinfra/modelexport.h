#pragma once

#include "infra/filesystem.h"
#include <qglobal.h>

QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)

namespace inf::ui {

// First shows a file selector to determine the file path
void exportModel(QWidget* parent, QAbstractItemModel* model, std::string_view name);
void exportModel(QAbstractItemModel* model, std::string_view name, fs::path outputPath);
}
