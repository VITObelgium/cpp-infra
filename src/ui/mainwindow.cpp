#include "mainwindow.h"

#include "preferencesdialog.h"
#include "runsimulationdialog.h"
#include "uiinfra/logview.h"

#include <qdockwidget.h>

namespace opaq {

MainWindow::MainWindow(const std::shared_ptr<QAbstractItemModel>& logSink, QWidget* parent)
: QMainWindow(parent)
, _logModel(logSink)
{
    _ui.setupUi(this);

    _ui.forecastPath->setFileSelectorFilter(tr("Config files (Forecast files (*.dat *.txt *.csv)"));
    _ui.forecastPath->setConfigName("RecentForecasts");
    _ui.forecastPath->setLabel(tr("Forecast data:"));

    setupDockWidgets();

    connect(_ui.actionQuit, &QAction::triggered, this, &QMainWindow::close);
    connect(_ui.actionPreferences, &QAction::triggered, this, &MainWindow::showPreferences);
    connect(_ui.forecastPath, &FileSelectionComboBox::pathChanged, this, &MainWindow::onForecastDataPathChanged);

    _ui.mapping->applyLegendSettings(PreferencesDialog().legendSettings());
}

void MainWindow::setupDockWidgets()
{
    _diagnoseDockWidget = new QDockWidget(tr("Diagnostics"), this);
    _diagnoseDockWidget->setObjectName("Diagnostics");
    _diagnoseDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea | Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    auto logView = new uiinfra::LogView(_diagnoseDockWidget);
    logView->setModel(_logModel.get());
    _diagnoseDockWidget->setWidget(logView);
    addDockWidget(Qt::BottomDockWidgetArea, _diagnoseDockWidget);
    _diagnoseDockWidget->hide();
    _diagnoseDockWidget->toggleViewAction()->setIcon(QIcon(QStringLiteral(":/data/bug.svg")));
    _diagnoseDockWidget->toggleViewAction()->setChecked(false);
    _diagnoseDockWidget->toggleViewAction()->setShortcut(Qt::CTRL + Qt::Key_D);
    _ui.menuView->addAction(_diagnoseDockWidget->toggleViewAction());
}

void MainWindow::showPreferences()
{
    PreferencesDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        dlg.saveSettings();

        _ui.mapping->applyLegendSettings(dlg.legendSettings());
    }
}

void MainWindow::onForecastDataPathChanged(const QString& path)
{
    _ui.mapping->setForecastDataPath(path.toStdString());
}

}
