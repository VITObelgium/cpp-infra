#pragma once

#include "infra/enumflags.h"
#include "infra/filesystem.h"
#include <qglobal.h>

QT_FORWARD_DECLARE_CLASS(QWidget)
QT_FORWARD_DECLARE_CLASS(QModelIndex)
QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)

namespace inf::ui {

enum ExportModelOptions
{
    NoVerticalHeaders   = 1,
    NoHorizontalHeaders = 2,
};

// First shows a file selector to determine the file path, returns false if the dialog was cancelled by the user
bool exportModel(QWidget* parent, const QAbstractItemModel* model, std::string_view name, Flags<ExportModelOptions> options = Flags<ExportModelOptions>());
bool exportModel(QWidget* parent, const QAbstractItemModel* model, const QModelIndex& rootIndex, std::string_view name, Flags<ExportModelOptions> options = Flags<ExportModelOptions>());

void exportModel(const QAbstractItemModel* model, std::string_view name, fs::path outputPath, Flags<ExportModelOptions> options = Flags<ExportModelOptions>());
void exportModel(const QAbstractItemModel* model, const QModelIndex& rootIndex, std::string_view name, fs::path outputPath, Flags<ExportModelOptions> options = Flags<ExportModelOptions>());
}
