#pragma once

#include "ui_mainwindow.h"

#include <QMainWindow>

class ToolBar;
QT_FORWARD_DECLARE_CLASS(QMenu)

namespace opaq {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef QMap<QString, QSize> CustomSizeHintMap;

    MainWindow();

private:
    Ui::MainWindow _ui;
};

}
