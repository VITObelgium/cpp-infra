#pragma once

#include "ui_mainwindow.h"

#include <QMainWindow>

QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)

namespace opaq {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const std::shared_ptr<QAbstractItemModel>& logSink, QWidget* parent = nullptr);

private:
    void setupDockWidgets();
    void showPreferences();

    void onForecastDataPathChanged(const QString& path);

    Ui::MainWindow _ui;
    std::shared_ptr<QAbstractItemModel> _logModel;
};
}
