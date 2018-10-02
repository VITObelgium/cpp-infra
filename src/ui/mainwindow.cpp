#include "mainwindow.h"

#include "runsimulationdialog.h"

#include <QMenuBar>

namespace opaq {

MainWindow::MainWindow()
: QMainWindow()
{
    _ui.setupUi(this);

    connect(_ui.actionQuit, &QAction::triggered, this, &QMainWindow::close);
}

}
