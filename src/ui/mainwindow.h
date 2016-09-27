#pragma once

#include <QMainWindow>

class ToolBar;
QT_FORWARD_DECLARE_CLASS(QMenu)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    typedef QMap<QString, QSize> CustomSizeHintMap;

    MainWindow();

public slots:
    void actionTriggered(QAction *action);

private:
    void setupToolBar();
    void setupMenuBar();

    QList<ToolBar*> toolBars;
    QMenu* mainWindowMenu;
};
