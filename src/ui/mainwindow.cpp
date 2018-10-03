#include "mainwindow.h"

#include "runsimulationdialog.h"
#include "uiinfra/logview.h"

#include <qdockwidget.h>

namespace opaq {

MainWindow::MainWindow(const std::shared_ptr<QAbstractItemModel>& logSink, QWidget* parent)
: QMainWindow(parent)
, _logModel(logSink)
{
    _ui.setupUi(this);

    setupDockWidgets();

    connect(_ui.actionQuit, &QAction::triggered, this, &QMainWindow::close);
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
    _diagnoseDockWidget->toggleViewAction()->setChecked(false);

    _ui.menuView->addAction(_diagnoseDockWidget->toggleViewAction());
    _diagnoseDockWidget->toggleViewAction()->setShortcut(Qt::CTRL + Qt::Key_D);
}

}
