#include "mainwindow.h"

#include "preferencesdialog.h"
#include "runsimulationdialog.h"
#include "uiinfra/logview.h"

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
    _ui.logView->setModel(_logModel.get());
    _ui.diagnoseDockWidget->toggleViewAction()->setShortcut(Qt::CTRL + Qt::Key_D);
    _ui.diagnoseDockWidget->hide();
    _ui.diagnoseDockWidget->toggleViewAction()->setChecked(false);
    _ui.diagnoseDockWidget->toggleViewAction()->setIcon(QIcon(QStringLiteral(":/data/bug.svg")));
    _ui.menuView->addAction(_ui.diagnoseDockWidget->toggleViewAction());
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
