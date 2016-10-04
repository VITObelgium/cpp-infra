#pragma once

#include "opaqview.h"

#include <QMainWindow>

class ToolBar;
QT_FORWARD_DECLARE_CLASS(QMenu)

namespace OPAQ
{

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef QMap<QString, QSize> CustomSizeHintMap;

    MainWindow();

private:
    void setupMenuBar();

    OpaqView _opaqView;
};

}
