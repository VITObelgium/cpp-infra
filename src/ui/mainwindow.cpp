#include "mainwindow.h"

#include "runsimulationdialog.h"

#include <QMenuBar>

namespace OPAQ
{

MainWindow::MainWindow()
: QMainWindow()
, _opaqView(this)
{
    setObjectName("Opaq");
    setWindowTitle("Opaq");
    setCentralWidget(&_opaqView);

    setupMenuBar();
}

void MainWindow::setupMenuBar()
{
    QMenu *menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(tr("&Quit"), this, &QWidget::close);
}

}
