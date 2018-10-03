#pragma once

#include "ui_mainwindow.h"

#include <QMainWindow>

QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)
QT_FORWARD_DECLARE_CLASS(QDockWidget)

namespace opaq {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const std::shared_ptr<QAbstractItemModel>& logSink, QWidget* parent = nullptr);

private:
    void setupDockWidgets();

    Ui::MainWindow _ui;
    std::shared_ptr<QAbstractItemModel> _logModel;
    QDockWidget* _diagnoseDockWidget = nullptr;
};
}
