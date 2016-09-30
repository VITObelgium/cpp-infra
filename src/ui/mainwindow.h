#pragma once

#include "predictionresultsmodel.h"
#include "ConfigurationHandler.h"
#include "Engine.h"
#include "opaqview.h"

#include <QMainWindow>
#include <QTableView>

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

public slots:
    void actionTriggered(QAction *action);

private:
    void setupToolBar();
    void setupMenuBar();

    void loadConfiguration();
    void runSimulation();
    void displayError(QString title, QString errorMsg);

    QList<ToolBar*> _toolBars;

    //QTableView _tableView;
    OpaqView _opaqView;

    Config::PollutantManager _pollutantMgr;
    ConfigurationHandler _config;
    Engine _engine;
    PredictionResultsModel _model;
};

}
