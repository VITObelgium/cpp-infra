#include "mainwindow.h"

#include "runsimulationdialog.h"
#include "AQNetwork.h"
#include "data/ForecastBuffer.h"

#include <QAction>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QFile>
#include <QDataStream>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSignalMapper>
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QDebug>
#include <QErrorMessage>

namespace OPAQ
{

MainWindow::MainWindow()
: QMainWindow()
, _tableView(this)
, _engine(_pollutantMgr)
, _model(this)
{
    setObjectName("Opaq");
    setWindowTitle("Opaq");
    setCentralWidget(&_tableView);

    setupToolBar();
    setupMenuBar();

    statusBar()->showMessage(tr("Status Bar"));

    _tableView.setModel(&_model);
}

void MainWindow::actionTriggered(QAction *action)
{
    qDebug("action '%s' triggered", action->text().toLocal8Bit().data());
}

void MainWindow::setupToolBar()
{
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
}

void MainWindow::setupMenuBar()
{
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    menu->addAction(tr("&Load configuration"), this, &MainWindow::loadConfiguration);
    menu->addAction(tr("&Quit"), this, &QWidget::close);

    QMenu *runMenu = menuBar()->addMenu(tr("&Run"));
    runMenu->addAction(tr("&Simulation"), this, &MainWindow::runSimulation);
}

void MainWindow::loadConfiguration()
{
    try
    {
        auto fileName = QFileDialog::getOpenFileName(this, tr("Load configuration"), "", tr("Config Files (*.xml)"));
        QFileInfo fileInfo(fileName);
        // Change the working directory to the config file so the relative paths can be found
        QDir::setCurrent(fileInfo.absoluteDir().path());
        _config.parseConfigurationFile(fileName.toStdString(), _pollutantMgr);
        _engine.prepareRun(_config.getOpaqRun());
    }
    catch (const std::exception& e)
    {
        displayError(tr("Failed to load config file"), e.what());
    }
}

void MainWindow::runSimulation()
{
    if (_config.getOpaqRun().getComponents().empty())
    {
        displayError(tr("Error"), tr("You first need to load a configuration"));
        return;
    }

    RunSimulationDialog dialog(_pollutantMgr.getList());
    if (dialog.exec() == QDialog::Accepted)
    {
        _config.getOpaqRun().setPollutantName(dialog.pollutant(), Aggregation::getName(dialog.aggregation()));

        auto basetime = dialog.basetime();
        if (!basetime.empty())
        {
            auto& basetimes = _config.getOpaqRun().getBaseTimes();
            basetimes.clear();

            try
            {
                auto baseTime = OPAQ::DateTimeTools::parseDate(basetime);
                int dayCount = 1;
                for (int i = 0; i < dayCount; ++i)
                {
                    basetimes.push_back(baseTime);
                    baseTime.addDays(1);
                }
            }
            catch (const OPAQ::ParseException&)
            {
                displayError(tr("Failed to parse base time"), QString("Failed to parse base time : %1").arg(basetime.c_str()));
                return;
            }
        }

        try
        {
            _config.validateConfiguration(_pollutantMgr);
            _engine.run(_config.getOpaqRun());

            auto& aqNetworkProvider = _engine.componentManager().getComponent<AQNetworkProvider>(_config.getOpaqRun().getNetworkProvider()->name);
            auto& buffer = _engine.componentManager().getComponent<ForecastBuffer>(_config.getOpaqRun().getForecastStage()->getBuffer().name);
            auto forecastHorizon = _config.getOpaqRun().getForecastStage()->getHorizon();
            
            _model.updateResults(buffer, dialog.basetime(), aqNetworkProvider.getAQNetwork()->getStations(), forecastHorizon, dialog.pollutant(), dialog.aggregation());
        }
        catch (const std::exception& e)
        {
            displayError(tr("Failed to run simulation"), e.what());
        }
    }
}

void MainWindow::displayError(QString title, QString errorMsg)
{
    QMessageBox messageBox;
    messageBox.critical(this, title, errorMsg);
}

}
